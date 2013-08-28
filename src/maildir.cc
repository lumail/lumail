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

#include <algorithm>
#include <dirent.h>
#include <iomanip>
#include <pcrecpp.h>
#include <sstream>
#include <vector>

#include "debug.h"
#include "file.h"
#include "global.h"
#include "maildir.h"
#include "message.h"


/**
 * Constructor.
 */
CMaildir::CMaildir(std::string path)
{
    m_path     = path;
    m_name     = "";
    m_modified = 0;   /* mtime of the maildir */
    m_unread   = -1;  /* unread messages in maildir */
    m_total    = -1;  /* total messages in maildir */

#ifdef LUMAIL_DEBUG
    std::string dm = "CMaildir::CMaildir(";
    dm += m_path;
    dm += ");";
    DEBUG_LOG( dm );
#endif
}

/**
 * Destructor.  NOP.
 */
CMaildir::~CMaildir()
{

#ifdef LUMAIL_DEBUG
    std::string dm = "CMaildir::~CMaildir(";
    dm += path();
    dm += ");";
    DEBUG_LOG( dm );
#endif

}


/**
 * Return the last modified time for this Maildir.
 */
time_t CMaildir::last_modified()
{
    time_t last = 0;
    struct stat st_buf;

    std::string p = path();

    /**
     * The two directories we care about: new/ + cur/
     */
    std::vector<std::string> dirs;
    dirs.push_back(p + "/cur");
    dirs.push_back(p + "/new");

    /**
     * See which was the most recently modified.
     */
    for (std::string dir : dirs)
    {
        /**
         * If we can stat() the dir and it is more recent
         * than the current value - update it.
         */
        if ( ! stat(dir.c_str(),&st_buf) )
            if ( st_buf.st_mtime > last )
                last = st_buf.st_mtime;
    }

    return( last );
}



/**
 * Update the cached total/unread message counts.
 */
void CMaildir::update_cache()
{
    /**
     * If the cached date isn't different then we need do nothing.
     */
    time_t last_mod = last_modified();

    /**
     * If we've got -1 for the count/unread then we've
     * just been created.
     *
     */
    if ( ( last_mod <= m_modified ) &&
         ( m_unread != -1 ) &&
         ( m_total != -1 ) )
        return;

    m_modified = last_mod;


    /**
     * Get all messages, and update the total
     */
    std::vector<CMessage *> all = getMessages();
    std::vector<CMessage *>::iterator it;
    m_total = all.size();


    /**
     * Now update the unread count.
     */
    m_unread = 0;
    for (it = all.begin(); it != all.end(); ++it)
    {
        if ( (*it)->is_new() )
            m_unread += 1;
    }

    /**
     * Now cleanup.
     */
    for (it = all.begin(); it != all.end(); ++it)
    {
        delete( *it );
    }
}


/**
 * The number of new messages for this maildir.
 */
int CMaildir::unread_messages()
{
    update_cache();
    return( m_unread );
}


/**
 * The total number of messages for this maildir.
 */
int CMaildir::total_messages()
{
    update_cache();
    return( m_total );
}


/**
 * The friendly name of the maildir.
 */
std::string CMaildir::name()
{
    /**
     * Since maildirs can never be renamed we can safely cache the
     * name locally.
     */
    if ( !m_name.empty() )
        return( m_name );

    /**
     * Find the last part of the name.
     */
    std::string p = path();

    size_t found = p.find_last_of("/");
    if ( found != std::string::npos )
        m_name = p.substr(found + 1);
    else
        m_name = p;

    return (m_name);
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
                int total = total_messages();
                convert << std::setfill('0') << std::setw(4) << total;
                result.insert(offset, convert.str());
            }
            if ( strcmp(std_name[i] , "$READ" ) == 0 )
            {
                int read = total_messages() - unread_messages();;
                convert << std::setfill('0') << std::setw(4) << read;
                result.insert(offset, convert.str());
            }
            if ( ( strcmp(std_name[i] , "$NEW" ) == 0 ) ||
                 ( strcmp(std_name[i] , "$UNREAD" ) == 0 ) )
            {
                int unread = unread_messages();
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
        if ( unread_messages() > 0)
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
 * Get each messages in the folder.
 *
 * These are heap-allocated and will be persistent until the folder
 * selection is changed.
 *
 * The return value is *all possible messages*, no attention to `index_limit`
 * is paid.
 *
 *  TODO:  Use CFile::files_in_directory().
 *
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

#ifdef LUMAIL_DEBUG
    std::string dm = "CMessage::getMessages()";
    DEBUG_LOG( dm );
#endif

    /**
     * For each directory.
     */
    for (std::string path : dirs)
    {
        dp = opendir(path.c_str());
        if (dp)
        {
            while (true)
            {
                de = readdir(dp);
                if (de == NULL)
                    break;

                /** Maybe we should check for DT_REG || DT_LNK ? */
                if ( (de->d_type != DT_DIR)
                   || ( de->d_type == DT_UNKNOWN && !CFile::is_directory (std::string(path + de->d_name))))
                {
                    CMessage *t = new CMessage(std::string(path + de->d_name));
                    result.push_back(t);

#ifdef LUMAIL_DEBUG
                    std::string dm = "CMessage::getMessages() - found ";
                    dm += path + de->d_name;
                    DEBUG_LOG( dm );
#endif
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

    char host[1024] = {'\0'};
    gethostname(host, sizeof(host)-1);
    std::string hostname( host );

    /**
     * Loop until we found a file that is unique.
     */
    while( true )
    {
        /**
         * Convert the seconds past the epoch to a string.
         */
        time_t current_time = time(NULL);
        std::stringstream ss;
        ss << current_time;
        std::string since_epoch = ss.str();

        std::string file = since_epoch;
        file += ".";
        file += hostname;

        /**
         * Random number.
         */
        int r = rand() % 1000;
        ss << r;
        file += ss.str();

        /**
         * Generate the temporary file.
         */
        file += ":2";

        if ( is_new )
            file += ",N";
        else
            file += ",S";

        if ( ! CFile::exists( path  + file ) )
            return( path + file );
    }
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

    for (std::string dir : dirs)
    {
        if (!CFile::is_directory(dir))
            return false;
    }
    return true;
}


/**
 * Create a new Maildir.
 *
 * NOTE: Parent directory/directories must exist.
 */
bool CMaildir::create(std::string path)
{
    /**
     * Get the parent.
     */
    size_t offset = path.find_last_of( "/" );
    if ( offset != std::string::npos )
    {
        std::string parent = path.substr(0,offset);

        /**
         * Does the parent exist as a directory?
         */
        if ( ! CFile::is_directory( parent ) )
            return false;

        /**
         * OK we're probably alright.  Create the directories.
         */
        std::vector<std::string> dirs;
        dirs.push_back(path);
        dirs.push_back(path + "/cur");
        dirs.push_back(path + "/new");
        dirs.push_back(path + "/tmp");

        for( std::string dir : dirs )
        {
            if (mkdir( dir.c_str(), 0775) != 0 )
                return false;
        }
        return true;
    }
    else
    {
        return false;
    }
}
