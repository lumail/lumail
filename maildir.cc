/**
 * maildir.cc - Utility functions for working with Maildirs
 *
 * This file is part of lumail: http://lumail.org/
 *
 * Copyright (c) 2013 by Steve Kemp.  All rights reserved.
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

#include <vector>
#include <algorithm>

#include <sstream>
#include <iomanip>
#include <sys/types.h>
#include <dirent.h>
#include <pcrecpp.h>

#include "file.h"
#include "global.h"
#include "maildir.h"
#include "message.h"

/**
 * Constructor.  NOP
 */
CMaildir::CMaildir(std::string path)
{
    m_path = path;
}

/**
 * Destructor.  NOP.
 */
CMaildir::~CMaildir()
{
}

/**
 * The number of new messages for this directory.
 */
int CMaildir::newMessages()
{
    /**
     * Get all messages.
     */
    std::vector<CMessage *> all = getMessages();

    /**
     * If unread .. add to the total.
     */
    int unread = 0;

    std::vector<CMessage *>::iterator it;
    for (it = all.begin(); it != all.end(); ++it)
    {
        if ( (*it)->is_new() )
            unread += 1;
    }

    /**
     * Now cleanup.
     */
    for (it = all.begin(); it != all.end(); ++it)
    {
        delete( *it );
    }

    return( unread );
}

/**
 * The total number of messages for this directory.
 */
int CMaildir::availableMessages()
{
    /**
     * Get all messages.
     */
    std::vector<CMessage *> all = getMessages();

    int total = all.size();

    /**
     * Now cleanup.
     */
    std::vector<CMessage *>::iterator it;
    for (it = all.begin(); it != all.end(); ++it)
    {
        delete( *it );
    }

    return( total );
}

/**
 * The friendly name of the maildir.
 */
std::string CMaildir::name()
{
    unsigned found = m_path.find_last_of("/");
    return (m_path.substr(found + 1));
}

/**
 * The full path to the folder.
 */
std::string CMaildir::path()
{
    return (m_path);
}


/**
 * Format this maildir for display in maildir-mode.
 */
std::string CMaildir::format( bool selected, std::string fmt )
{
    std::string result;

    /**
     * Get the format-string we'll expand from the global
     * setting, if it wasn't supplied.
     */
    if ( fmt.empty() )
    {
        CGlobal *global  = CGlobal::Instance();
        std::string *fmt = global->get_variable("maildir_format");
        result = std::string(*fmt);
    }
    else
    {
        result = fmt;
    }


    /**
     * The variables we know about.
     */
    const char *fields[8] = { "$CHECK",
                              "$TOTAL",
                              "$READ",
                              "$NEW",
                              "$UNREAD",
                              "$PATH",
                              "$NAME",
                              0 };
    const char **std_name = fields;


    /**
     * Iterate over everything we could possibly-expand.
     */
    for( int i = 0 ; std_name[i] ; ++i)
    {
        size_t offset = result.find( std_name[i], 0 );

        if ( ( offset != std::string::npos ) && ( offset < result.size() ) )
        {

            /**
             * Remove the part we don't care about.
             */
            result.erase( offset, strlen( std_name[i] ) );

            /**
             * Conversion helper.
             */
            std::ostringstream convert;

            /**
             * Expand the specific variables.
             */
            if ( strcmp(std_name[i] , "$CHECK" ) == 0 )
            {
                if ( selected )
                    result.insert(offset, "[X]" );
                else
                    result.insert(offset, "[ ]" );
            }
            if ( strcmp(std_name[i] , "$TOTAL" ) == 0 )
            {
                int total = availableMessages() + newMessages();
                convert << std::setfill('0') << std::setw(4) << total;
                result.insert(offset, convert.str());
            }
            if ( strcmp(std_name[i] , "$READ" ) == 0 )
            {
                int read = availableMessages() - newMessages();;
                convert << std::setfill('0') << std::setw(4) << read;
                result.insert(offset, convert.str());
            }
            if ( ( strcmp(std_name[i] , "$NEW" ) == 0 ) ||
                 ( strcmp(std_name[i] , "$UNREAD" ) == 0 ) )
            {
                int unread = newMessages();
                convert << std::setfill('0') << std::setw(4) << unread;
                result.insert(offset, convert.str());
            }
            if ( strcmp(std_name[i] , "$PATH" ) == 0 )
            {
                result.insert(offset, path());
            }
            if ( strcmp(std_name[i] , "$NAME" ) == 0 )
            {
                result.insert(offset, name());
            }
        }
    }
    return( result );

}


/**
 * Does this folder match the given filter.
 */
bool CMaildir::matches_filter( std::string *filter )
{
    if (strcmp(filter->c_str(), "all") == 0)
        return true;

    if (strcmp(filter->c_str(), "new") == 0)
    {
        if ( newMessages() > 0)
            return true;
        else
            return false;
    }

    std::string p = path();

    /**
     * Regexp Matching.
     */
    if (pcrecpp::RE(*filter, pcrecpp::RE_Options().set_caseless(true)).PartialMatch(p) )
        return true;

    return false;
}

/**
 * Count files in a directory.
 */
int CMaildir::countFiles(std::string path)
{
    int count = 0;
    dirent *de;
    DIR *dp;

    dp = opendir(path.c_str());
    if (dp)
    {
        while (true)
        {
            de = readdir(dp);
            if (de == NULL)
                break;

            if (!CFile::is_directory(std::string(path + "/" + de->d_name)))
                count += 1;
        }
        closedir(dp);
    }
    return count;
}


/**
 * Return a sorted list of maildirs beneath the given path.
 */
std::vector<std::string> CMaildir::getFolders(std::string path)
{
    std::vector<std::string> result;
    dirent *de;
    DIR *dp;

    std::string prefix = path.empty()? "." : path.c_str();
    dp = opendir(prefix.c_str());
    if (dp)
    {
        while (true)
        {
            de = readdir(dp);
            if (de == NULL)
                break;

            if ( ( strcmp( de->d_name, "." ) != 0 ) &&
                 ( strcmp( de->d_name, ".." ) != 0 ) &&
		 ( ( de->d_type == DT_UNKNOWN) || 
		   ( de->d_type == DT_DIR ) ) )
            {
                std::string subdir_name = std::string(de->d_name);
                std::string subdir_path = std::string(prefix + "/" + subdir_name);
                if (CMaildir::is_maildir(subdir_path))
                    result.push_back(subdir_path);
                else
                {
                    if (subdir_name != "." && subdir_name != "..")
                    {
                        DIR* sdp = opendir(subdir_path.c_str());
                        if (sdp)
                        {
                            closedir(sdp);
                            std::vector<std::string> sub_maildirs;
                            sub_maildirs = CMaildir::getFolders(subdir_path);
                            std::vector<std::string>::iterator it;
                            for (it = sub_maildirs.begin(); it != sub_maildirs.end(); ++it)
                            {
                                result.push_back(*it);
                            }
                        }
                    }
                }
            }
        }
        closedir(dp);
        std::sort(result.begin(), result.end());
    }
    return result;
}

/**
 * Get each messages in the folder.
 *
 * These are heap-allocated and will be persistent until the folder
 * selection is changed.
 *
 * The return value is *all possible messages*, no attention to `index_limit`
 * is paid.
 */
std::vector<CMessage *> CMaildir::getMessages()
{
    std::vector<CMessage*> result;
    dirent *de;
    DIR *dp;

    /**
     * Directories we search.
     */
    std::vector<std::string> dirs;
    dirs.push_back(m_path + "/cur/");
    dirs.push_back(m_path + "/new/");

    /**
     * For each directory.
     */
    std::vector<std::string>::iterator it;
    for (it = dirs.begin(); it != dirs.end(); ++it)
    {

        std::string path = *it;

        dp = opendir(path.c_str());
        if (dp)
        {

            while (true)
            {
                de = readdir(dp);
                if (de == NULL)
                    break;

                if (!CFile::is_directory (std::string(path + de->d_name)))
                {
                    CMessage *t = new CMessage(std::string(path + de->d_name));
                    result.push_back(t);
                }
            }
            closedir(dp);
        }
    }
    return result;
}


/**
 * Generate a new filename in the given folder.
 */
std::string CMaildir::message_in(std::string path, bool is_new)
{
    /**
     * Ensure the path is a maildir.
     */
    if (! CMaildir::is_maildir(path) )
        return "";

    /**
     * Generate the path.
     */
    if ( is_new )
        path += "/new/";
    else
        path += "/cur/";


    /**
     * Filename is: $time.xxx.$hostname.
     */
    std::string hostname = mimetic::gethostname();
    time_t current_time = time(NULL);

    /**
     * Convert the seconds to a string.
     */
    std::stringstream ss;
    ss << current_time;
    std::string since_epoch = ss.str();

    path += since_epoch;
    path += ".";
    path += hostname;

    /**
     * Generate the temporary file.
     */
    path += ":2";

    if ( is_new )
        path += ",N";
    else
        path += ",S";

    return( path );
}

/**
 * Is the given path a Maildir?
 */
bool CMaildir::is_maildir(std::string path)
{
    std::vector<std::string> dirs;
    dirs.push_back(path);
    dirs.push_back(path + "/cur");
    dirs.push_back(path + "/tmp");
    dirs.push_back(path + "/new");

    std::vector<std::string>::iterator it;
    for (it = dirs.begin(); it != dirs.end(); ++it)
    {
        if (!CFile::is_directory(*it))
            return false;
    }
    return true;
}
