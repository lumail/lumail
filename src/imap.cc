/*
 * imap.cc - Simple IMAP handling.
 *
 * This file is part of lumail - http://lumail.org/
 *
 * Copyright (c) 2015 by Steve Kemp.  All rights reserved.
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


/**
 * This is horrid.
 */
static std::string m_txt;


CIMAP::CIMAP()
{
    m_ssl_verify = false;
    m_curl = curl_easy_init();

    if (!m_curl)
    {
        fprintf(stderr, "Failed to init curl\n");
        exit(-1);
    }
}


/**
 * Destructor.
 */
CIMAP::~CIMAP()
{
    curl_easy_cleanup(m_curl);
};


/**
 * Setup login-credentials.
 */
void CIMAP::set_password(std::string password)
{
    m_password = password;
}

/**
 * Setup login-credentials.
 */
void CIMAP::set_username(std::string username)
{
    m_username = username;
}

/**
 * Setup the remote mail-server name, as an URL,
 * such as `imaps://mail.example.com/`
 */
void CIMAP::set_server(std::string server)
{
    m_server = server;
};


/**
 * Return a list of the folders available.
 */
std::vector<std::string> CIMAP::getMaildirs()
{
    std::vector<std::string> result;
    m_txt = "";

    /* Set username and password */
    curl_easy_setopt(m_curl, CURLOPT_USERNAME, m_username.c_str());
    curl_easy_setopt(m_curl, CURLOPT_PASSWORD, m_password.c_str());
    curl_easy_setopt(m_curl, CURLOPT_URL, m_server.c_str());

    if (m_ssl_verify == false)
    {
        curl_easy_setopt(m_curl, CURLOPT_SSL_VERIFYHOST, 0);
        curl_easy_setopt(m_curl, CURLOPT_SSL_VERIFYPEER, 0);
    }

    curl_easy_setopt(m_curl, CURLOPT_WRITEFUNCTION, WriteMemoryCallback);
    CURLcode res = curl_easy_perform(m_curl);

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

    return (result);


}

int CIMAP::count_unread(std::string folder)
{
    m_txt = "";

    folder = urlencode(folder);
    std::string path = m_server + folder;

    fprintf(stderr, "count_unread('%s')\n",
            path.c_str());


    /* Set username and password */
    curl_easy_setopt(m_curl, CURLOPT_USERNAME, m_username.c_str());
    curl_easy_setopt(m_curl, CURLOPT_PASSWORD, m_password.c_str());

    if (m_ssl_verify == false)
    {
        curl_easy_setopt(m_curl, CURLOPT_SSL_VERIFYHOST, 0);
        curl_easy_setopt(m_curl, CURLOPT_SSL_VERIFYPEER, 0);
    }

    curl_easy_setopt(m_curl, CURLOPT_URL, path.c_str());
    curl_easy_setopt(m_curl, CURLOPT_WRITEFUNCTION, WriteMemoryCallback);
    curl_easy_setopt(m_curl, CURLOPT_CUSTOMREQUEST, "SEARCH UNSEEN");

    CURLcode res = curl_easy_perform(m_curl);

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

    fprintf(stderr, "COUNT UNREAD - '%s' - LINE - '%s'\n",
            folder.c_str(), m_txt.c_str());

    if (m_txt.length() > strlen("* RESULT"))
        m_txt = m_txt.substr(strlen("* RESULT"));

    // No results?  Finish.
    if (m_txt.empty())
        return 0;
    else
        m_txt = m_txt.substr(1);

    // Otherwise count the spaces and return.
    size_t n = std::count(m_txt.begin(), m_txt.end(), ' ');
    return (n + 1);
}


int CIMAP::count_total(std::string folder)
{
    m_txt = "";

    /* Set username and password */
    curl_easy_setopt(m_curl, CURLOPT_USERNAME, m_username.c_str());
    curl_easy_setopt(m_curl, CURLOPT_PASSWORD, m_password.c_str());
    curl_easy_setopt(m_curl, CURLOPT_URL, m_server.c_str());

    if (m_ssl_verify == false)
    {
        curl_easy_setopt(m_curl, CURLOPT_SSL_VERIFYHOST, 0);
        curl_easy_setopt(m_curl, CURLOPT_SSL_VERIFYPEER, 0);
    }

    std::string path = "EXAMINE \"";
    path += folder;
    path += "\"";

    curl_easy_setopt(m_curl, CURLOPT_WRITEFUNCTION, WriteMemoryCallback);
    curl_easy_setopt(m_curl, CURLOPT_CUSTOMREQUEST, path.c_str());

    CURLcode res = curl_easy_perform(m_curl);

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
        fprintf(stderr, "COUNT_TOTAL PATH - '%s' - LINE - '%s'\n",
                folder.c_str(), m_txt.c_str());


        std::vector<std::string> x = split(line, ' ');

        if ((x.size() == 3) &&
                (x.at(2) == "EXISTS"))
        {
            count = atoi(x.at(1).c_str());
        }
    }

    return (count);
}

/**
 * This function is called whenever new data is received as a result
 * of an URL-fetch.
 *
 * We append the new data to the global/static m_txt member.
 */
size_t CIMAP::WriteMemoryCallback(void *ptr, size_t size, size_t nmemb, void *data)
{
    size_t realsize = size * nmemb;
    m_txt.append((char *)ptr, realsize);

    return realsize;
};
