/*
 * imap.cc - Simple IMAP handling.
 *
 * This file is part of lumail - http://lumail.org/
 *
 * Copyright (c) 2016 by Steve Kemp.  All rights reserved.
 *
 **
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; version 2 dated June, 1991, or (at your
 * option) any later version.
 *
 * On Debian GNU/Linux systems, the complete text of version 2 of the GNU
 * General Public License can be found in `/usr/share/common-licenses/GPL-2'
 */

#include <stdio.h>
#include <stdlib.h>
#include <malloc.h>
#include <string.h>
#include <curl/curl.h>

#include <algorithm>
#include <sstream>
#include <vector>
#include <string>

#include "imap.h"


CIMAP::CIMAP()
{
    m_ssl_verify = false;
    m_txt        = "";
}


/**
 * Destructor.
 */
CIMAP::~CIMAP()
{
}


/*
 * Setup login-credentials.
 */
void CIMAP::set_password(std::string password)
{
    m_password = password;
}


/*
 * Setup login-credentials.
 */
void CIMAP::set_username(std::string username)
{
    m_username = username;
}


/*
 * Setup the remote mail-server name, as an URL,
 * such as `imaps://mail.example.com/`
 */
void CIMAP::set_server(std::string server)
{
    m_server = server;
};


/*
 * Return a list of the available folders.
 */
std::vector<std::string> CIMAP::getMaildirs()
{
    std::vector<std::string> result;

    /* Reset our state */
    m_txt      = "";
    CURL *curl = curl_easy_init();


    /* Set username and password */
    curl_easy_setopt(curl, CURLOPT_USERNAME, m_username.c_str());
    curl_easy_setopt(curl, CURLOPT_PASSWORD, m_password.c_str());
    curl_easy_setopt(curl, CURLOPT_URL, m_server.c_str());

    if (m_ssl_verify == false)
    {
        curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 0);
        curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0);
    }

    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteMemoryCallback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void *)(this));

    CURLcode res = curl_easy_perform(curl);

    /* Check for errors */
    if (res != CURLE_OK)
    {
        fprintf(stderr, "curl_easy_perform() failed: %s\n",
                curl_easy_strerror(res));
        exit(1);
    }


    /* If we reached here our string is populated.  split on lineline */
    std::stringstream ss(m_txt);
    std::string line;

    while (std::getline(ss, line, '\n'))
    {

        line.erase(std::remove(line.begin(), line.end(), '\n'), line.end());
        line.erase(std::remove(line.begin(), line.end(), '\r'), line.end());

        //
        // Create a downcased version of the line.
        //
        std::string l(line);
        std::transform(l.begin(), l.end(), l.begin(), tolower);

        //
        //  Skip this line if we find the `noselect` string inside it
        //
        // This is found, for example, in output like this:
        //
        // * LIST ..
        // * LIST (\HasChildren \Noselect) "/" "[Google Mail]"
        // * LIST (\All \HasNoChildren) "/" "[Google Mail]/All Mail"
        // * LIST (\Drafts \HasNoChildren) "/" "[Google Mail]/Drafts"
        // * LIST ..
        //
        size_t found = l.find("noselect");

        if (found != std::string::npos)
            continue;

        // Lines returned will have the form:
        //
        //   * LIST (\HasNoChildren) "/" people-figment
        //   * LIST (\HasNoChildren) "/" people-harry
        //   * LIST (\HasNoChildren) "/" people-steve.backup.kemp
        //   * LIST (\HasNoChildren) "/" "Google/.edinburgh-mail/[Gmail]/.Sent Mail"
        //
        // So we cannot split on the last space, we have to look
        // to see if the folder is quoted - then work backwards.
        //
        //   * If the last character is " then we know the name
        //     is between the last two quotes.
        //
        //   * Otherwise look for 'quote space' and then use the name
        //     after that.
        //
        if (line.empty())
        {
            // Can't happen?
        }
        else if (line.at(line.length() - 1) == '"')
        {

            // Remove the last quote.
            line = line.substr(0, line.length() - 1);

            // Now look for the *opening* quote.
            std::size_t found = line.find_last_of("\"");

            // And we've found the mailbox-name
            line = line.substr(found + 1);
            result.push_back(line);
        }
        else
        {
            //
            // Otherwise we're lookign for "* LIST (XXX) "/" XXXX
            //
            // Find the last quote - which will terminate the "/" section.
            //
            std::size_t found = line.find_last_of("\"");

            //
            // And the next thing is the mailbox name.
            //
            if (found != std::string::npos)
            {
                line = line.substr(found + 2);
                result.push_back(line);
            }
        }
    }

    //
    // Sort the results.
    //
    std::sort(result.begin(), result.end());

    /* Cleanup. */
    curl_easy_cleanup(curl);
    return (result);
}


/*
 * Count the unread-messages in the specified folder.
 */
int CIMAP::count_unread(std::string folder)
{
    /* reset our state */
    m_txt = "";
    CURL *curl = curl_easy_init();

    folder = urlencode(folder);
    std::string path = m_server + folder;

    /* Set username and password */
    curl_easy_setopt(curl, CURLOPT_USERNAME, m_username.c_str());
    curl_easy_setopt(curl, CURLOPT_PASSWORD, m_password.c_str());

    if (m_ssl_verify == false)
    {
        curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 0);
        curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0);
    }

    curl_easy_setopt(curl, CURLOPT_URL, path.c_str());
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteMemoryCallback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void *)(this));

    curl_easy_setopt(curl, CURLOPT_CUSTOMREQUEST, "SEARCH UNSEEN");

    CURLcode res = curl_easy_perform(curl);

    /* Check for errors */
    if (res != CURLE_OK)
    {
        fprintf(stderr, "curl_easy_perform() failed: %s\n",
                curl_easy_strerror(res));
        exit(1);
    }


    // The result will be something like:
    //
    //   * RESULT 1 2 3 4
    //
    // Skip the "* RESULT " and then count the spaces to see how
    // many messages exist.  In the example above we'd have:
    //
    //   1 2 3 4
    //
    // So three spaces == four entries.
    //
    // Remove linefeeds and the prefix.
    m_txt.erase(std::remove(m_txt.begin(), m_txt.end(), '\n'), m_txt.end());
    m_txt.erase(std::remove(m_txt.begin(), m_txt.end(), '\r'), m_txt.end());

    if ((m_txt == "* SEARCH") ||
            (m_txt == "* RESULT"))
    {
        curl_easy_cleanup(curl);
        return 0;
    }

    // Remove the prefix.
    m_txt = m_txt.substr(strlen("* RESULT"));

    // No results?  Finish.
    if (m_txt.empty())
    {
        curl_easy_cleanup(curl);
        return 0;
    }
    else
        m_txt = m_txt.substr(1);

    curl_easy_cleanup(curl);

    // Otherwise count the spaces and return.
    size_t n = std::count(m_txt.begin(), m_txt.end(), ' ');
    return (n + 1);
}


/*
 * Count the total-messages in the specified folder.
 */
int CIMAP::count_total(std::string folder)
{
    /* reset our state */
    m_txt = "";
    CURL *curl = curl_easy_init();

    /* Set username and password */
    curl_easy_setopt(curl, CURLOPT_USERNAME, m_username.c_str());
    curl_easy_setopt(curl, CURLOPT_PASSWORD, m_password.c_str());
    curl_easy_setopt(curl, CURLOPT_URL, m_server.c_str());

    if (m_ssl_verify == false)
    {
        curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 0);
        curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0);
    }

    std::string path = "EXAMINE \"";
    path += folder;
    path += "\"";

    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteMemoryCallback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void *)(this));

    curl_easy_setopt(curl, CURLOPT_CUSTOMREQUEST, path.c_str());

    CURLcode res = curl_easy_perform(curl);

    /* Check for errors */
    if (res != CURLE_OK)
    {
        fprintf(stderr, "curl_easy_perform() failed: %s\n",
                curl_easy_strerror(res));
        exit(1);
    }

    // The result will be something like:
    //
    //   * BLAH
    //   * 1234 TOTAL
    //   * BLAH
    //
    // Remove linefeeds and the prefix.

    /* If we reached here our string is populated.  split on lineline */
    std::stringstream ss(m_txt);
    std::string line;
    int count = 0;

    while (std::getline(ss, line, '\n'))
    {
        line.erase(std::remove(line.begin(), line.end(), '\n'), line.end());
        line.erase(std::remove(line.begin(), line.end(), '\r'), line.end());

        std::vector<std::string> x = split(line, ' ');

        if ((x.size() == 3) &&
                (x.at(2) == "EXISTS"))
        {
            count = atoi(x.at(1).c_str());
        }
    }

    curl_easy_cleanup(curl);
    return (count);
}

/**
 * Fetch the specified message.
 */
std::string CIMAP::fetch_message(std::string folder, int number)
{
    std::vector<std::string> result;

    /* Reset our state */
    m_txt      = "";
    CURL *curl = curl_easy_init();


    /* The path we'll request */
    std::string path = m_server;
    path += folder;
    path += ";UID=";
    path +=  std::to_string(number);


    /* Set username and password */
    curl_easy_setopt(curl, CURLOPT_USERNAME, m_username.c_str());
    curl_easy_setopt(curl, CURLOPT_PASSWORD, m_password.c_str());
    curl_easy_setopt(curl, CURLOPT_URL, path.c_str());

    if (m_ssl_verify == false)
    {
        curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 0);
        curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0);
    }

    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteMemoryCallback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void *)(this));

    CURLcode res = curl_easy_perform(curl);

    /* Check for errors */
    if (res != CURLE_OK)
    {
        fprintf(stderr, "curl_easy_perform() failed: %s\n",
                curl_easy_strerror(res));
        exit(1);
    }

    /* Cleanup. */
    curl_easy_cleanup(curl);
    return (m_txt);
}



/*
 * This function is called whenever new data is received as a result
 * of an URL-fetch.
 *
 * We append the new data to the m_txt member-variable.
 */
size_t CIMAP::WriteMemoryCallback(void *ptr, size_t size, size_t nmemb, void *obj_ptr)
{
    CIMAP *obj = (CIMAP*)obj_ptr;

    size_t realsize = size * nmemb;
    obj->m_txt.append((char *)ptr, realsize);

    return realsize;
};
