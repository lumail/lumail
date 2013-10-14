/**
 * global.cc - Singleton interface to store global data
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
#include <cstdlib>
#include <fstream>
#include <iostream>
#include <pcrecpp.h>
#include <stdlib.h>
#include <string.h>


#include "debug.h"
#include "file.h"
#include "global.h"
#include "lua.h"
#include "maildir.h"
#include "message.h"


/**
 * Instance-handle.
 */
CGlobal *CGlobal::pinstance = NULL;

/**
 * Get access to our singleton-object.
 */
CGlobal *CGlobal::Instance()
{
    if (!pinstance)
        pinstance = new CGlobal;

    return pinstance;
}

/**
 * Constructor - This is private as this class is a singleton.
 */
CGlobal::CGlobal()
{
    /**
     * Defaults.
     */
    m_cur_folder     = 0;
    m_cur_message    = 0;
    m_msg_offset     = 0;
    m_messages       = NULL;
    m_maildirs       = NULL;

    /**
     * Defaults as set in our variable hash-map.
     */
    set_variable( "completion_chars",       new std::string("'\"( ,") );
    set_variable( "display_filter",         new std::string("") );
    set_variable( "editor",                 new std::string("/usr/bin/vim") );
    set_variable( "global_mode",            new std::string("maildir"));
    set_variable( "history_file",           new std::string( "" ) );
    set_variable( "index_format",           new std::string( "[$FLAGS] $FROM - $SUBJECT" ) );
    set_variable( "index_highlight_mode",   new std::string( "standout" ) );
    set_variable( "index_limit",            new std::string("all") );
    set_variable( "mail_filter",            new std::string("") );
    set_variable( "maildir_format",         new std::string( "$CHECK - $PATH" ) );
    set_variable( "maildir_highlight_mode", new std::string( "standout" ) );
    set_variable( "maildir_limit",          new std::string("all") );
    set_variable( "sendmail_path",          new std::string( "/usr/lib/sendmail -t" ) );
    set_variable( "bounce_path",            new std::string( "/usr/lib/sendmail" ) );
    set_variable( "sort",                   new std::string( "date-asc" ) );

    /**
     * Default colours.
     */
    set_variable( "attachment_colour",     new std::string( "white" ) );
    set_variable( "body_colour",           new std::string( "white" ) );
    set_variable( "header_colour",         new std::string( "white" ) );
    set_variable( "unread_maildir_colour", new std::string( "red" ) );
    set_variable( "unread_message_colour", new std::string( "red" ) );

    /**
     * From address is a little fiddly.
     */
    std::string user = "UNKNOWN";
    if ( getenv( "USER" ) )
        user = getenv( "USER" );
    std::string *from = new std::string( user + "@localhost" );
    set_variable( "from", from );

    /**
     * Set our temporary directory.
     *
     * $TMPDIR, $TMP, then /tmp.
     */
    char const* tmp = getenv( "TMPDIR" );
    if ( tmp == NULL )
        tmp = getenv( "TMP" );

    if ( tmp != NULL )
        set_variable( "tmp", new std::string( tmp ) );
    else
        if ( CFile::is_directory( "/tmp" ) )
             set_variable( "tmp", new std::string( "/tmp" ) );




}



/*****
 *
 * Static sorting routines.  Not exported or visible outside this unit.
 *
 */


/**
 * Sort CMessages, by the user-selected criterion.
 */
bool sort_messages(CMessage *a, CMessage *b)
{
    assert( NULL != a );
    assert( NULL != b );

    /**
     * Get the type of sort.
     */
    CGlobal   *global = CGlobal::Instance();
    std::string *sort = global->get_variable("sort");

    /**
     * Sort by date.  Default if nothing is explicitly set.
     */
    if ( ( sort->empty() ) ||
         ( strcmp( sort->c_str(), "date-asc" ) == 0 ) ||
         ( strcmp( sort->c_str(), "date-desc" ) == 0 ) )
    {
        bool asc = true;

        /**
         * Are we descending?
         */
        if  ( ( ! sort->empty() ) && ( strcmp( sort->c_str(), "date-desc" ) == 0 ) )
            asc = false;

        if ( asc )
            return (a->mtime() < b->mtime());
        else
            return (b->mtime() < a->mtime());

    }

    /**
     * Sort by subject, asc/desc.
     */
    if (  ( !sort->empty() ) &&
          ( ( strcmp(sort->c_str(), "subject" ) == 0 )  ||
            ( strcmp(sort->c_str(), "subject-asc" ) == 0 )  ||
            ( strcmp(sort->c_str(), "subject-desc" ) == 0 )  ) )
    {

        bool asc = true;

        /**
         * Are we descending?
         */
        if  ( ( ! sort->empty() ) && ( strcmp( sort->c_str(), "subject-desc" ) == 0 ) )
            asc = false;

        std::string as = a->header("Subject");
        std::string bs = b->header("Subject");

        if ( ! asc )
        {
            std::string tmp = as;
            as = bs;
            bs = tmp;
        }

        std::transform(as.begin(), as.end(), as.begin(), tolower);
        std::transform(bs.begin(), bs.end(), bs.begin(), tolower);

        return( strcmp( as.c_str(), bs.c_str() ) < 0 );
    }

    /**
     * Sort by sender, asc/desc.
     */
    if (  ( !sort->empty() ) &&
          ( ( strcmp(sort->c_str(), "from" ) == 0 )  ||
            ( strcmp(sort->c_str(), "from-asc" ) == 0 )  ||
            ( strcmp(sort->c_str(), "from-desc" ) == 0 )  ) )
    {

        bool asc = true;

        /**
         * Are we descending?
         */
        if  ( ( sort != NULL ) && ( strcmp( sort->c_str(), "from-desc" ) == 0 ) )
            asc = false;

        std::string as = a->header("From");
        std::string bs = b->header("From");

        if ( ! asc )
        {
            std::string tmp = as;
            as = bs;
            bs = tmp;
        }

        std::transform(as.begin(), as.end(), as.begin(), tolower);
        std::transform(bs.begin(), bs.end(), bs.begin(), tolower);

        return( strcmp( as.c_str(), bs.c_str() ) < 0 );
    }


    /**
     * Sort by the date-header
     */
    if (  ( ! sort->empty() ) &&
          ( ( strcmp(sort->c_str(), "header" ) == 0 )  ||
            ( strcmp(sort->c_str(), "header-asc" ) == 0 )  ||
            ( strcmp(sort->c_str(), "header-desc" ) == 0 )  ) )
    {

        bool asc = true;

        /**
         * Are we descending?
         */
        if  ( ( sort != NULL ) && ( strcmp( sort->c_str(), "header-desc" ) == 0 ) )
            asc = false;

        time_t at = a->get_date_field();
        time_t bt = b->get_date_field();

        if ( asc )
            return ( at < bt );
        else
            return ( bt < at );

    }

    return false;
}


/**
 * Sort maildirs by name, case-insensitively.
 */
bool sort_maildir_ptr_by_name(CMaildir *a, CMaildir *b)
{
    assert( NULL != a );
    assert( NULL != b );

    /**
     * Sort case-insesitively.
     */
    return( strcasecmp(a->path().c_str(), b->path().c_str() ) < 0 );
}


/**
 * Get all selected folders.
 */
std::vector<std::string> CGlobal::get_selected_folders()
{
    return (m_selected_folders);
}


/**
 * Get folders matching the current mode.
 */
std::vector<CMaildir *> CGlobal::get_folders()
{
    CGlobal *global = CGlobal::Instance();
    std::vector<CMaildir*> display;
    std::string * filter = global->get_variable("maildir_limit");


    /**
     * If we have no folders then we must return the empty set.
     *
     * Most likely cause?  The maildir_prefix isn't set, or is set incorrectly.
     */
    if ( m_maildirs == NULL )
        return( display );


    /**
     * Filter the folders to those we can display
     */
    for (CMaildir *maildir : (*m_maildirs))
    {
        if ( maildir->matches_filter( filter ) )
            display.push_back(maildir);
    }

    /**
     * Sort, case-insensitively.
     */
    std::sort(display.begin(), display.end(), sort_maildir_ptr_by_name);

    return (display);
}


/**
 * Get all messages from the currently selected folders.
 */
std::vector<CMessage *>* CGlobal::get_messages()
{
    return( m_messages );
}


/**
 * Update the list of global Maildirs.
 */
void CGlobal::update_maildirs()
{
    DEBUG_LOG( "CGlobal::update_maildir" );

    /**
     * If we have items already then free each of them.
     */
    if ( m_maildirs != NULL )
    {
        /**
         * Delete
         */
        for (CMaildir *maildir : (*m_maildirs))
        {
            delete( maildir );
        }
        delete( m_maildirs );
        m_maildirs = NULL;
    }


    /**
     * Create a new vector to hold the results.
     */
    m_maildirs = new std::vector<CMaildir *>;


    /**
     * Get the maildir-prefix, if this is empty we cannot find any maildirs,
     * and we should thus return the empty set.
     */
    CGlobal *global     = CGlobal::Instance();
    std::string *prefix = global->get_variable( "maildir_prefix" );
    if ( prefix == NULL )
        return;


    /**
     * Get each maildir
     */
    std::vector<std::string> folders = CFile::get_all_maildirs(*prefix);

    /**
     * Should we ignore folders?
     */
    CLua *lua = CLua::Instance();
    std::vector<std::string> ignored = lua->table_to_array( "ignored_folders" );

    for (std::string path : folders)
    {
        bool ignore = false;

        /**
         * Should we ignore this folder?
         */
        if ( ignored.size() > 0 )
        {
            /**
             *  Iterate over all ignored paths.
             */
            for (std::string reg : ignored)
            {
                /**
                 * Ignore empty strings.
                 */
                if ( reg.size() < 1 )
                    break ;

                /**
                 * Perform the regex matching, via PCRE.
                 */
                if (pcrecpp::RE(reg, pcrecpp::RE_Options().set_caseless(true)).PartialMatch(path) )
                    ignore = true;
            }

        }

        /**
         * Not ignoring anything.  Add the folder.
         */
        if ( ! ignore )
            m_maildirs->push_back(new CMaildir(path));
    }

    /**
     * Sort, case-insensitively.
     */
    std::sort(m_maildirs->begin(), m_maildirs->end(), sort_maildir_ptr_by_name);

}


/**
 * Update the list of global messages, using the index_limit string set by lua.
 */
void CGlobal::update_messages()
{

    DEBUG_LOG( "CGlobal::update_messages" );


    /**
     * If we have items already then free each of them.
     */
    if ( m_messages != NULL )
    {
        for (CMessage *message : (*m_messages) )
        {
            delete( message );
        }
        delete( m_messages );
        m_messages = NULL;
    }

    /**
     * create a new store.
     */
    m_messages = new std::vector<CMessage *>;


    /**
     * Get the selected maildirs.
     */
    CGlobal *global = CGlobal::Instance();
    std::vector<std::string> folders = global->get_selected_folders();
    std::string * filter = global->get_variable("index_limit" );


    /**
     * For each selected maildir read the messages.
     */
    for (std::string folder : folders)
    {
        /**
         * get the messages from this folder.
         */
        CMaildir tmp = CMaildir(folder);
        std::vector<CMessage *> contents = tmp.getMessages();

        /**
         * Append to the list of messages combined.
         */
        for (CMessage *content: contents)
        {
            if ( content->matches_filter( filter ) )
                m_messages->push_back(content) ;
        }

        /**
         * Sort?
         */
        std::sort(m_messages->begin(), m_messages->end(), sort_messages);
    }


}

/**
 * Remove all selected folders.
 */
void CGlobal::unset_folders()
{
    m_selected_folders.clear();
    assert( m_selected_folders.size() == 0 );
}

/**
 * Add a folder to the selected set.
 */
void CGlobal::add_folder(std::string path)
{
    m_selected_folders.push_back(path);
    assert( m_selected_folders.size() > 0 );
}

/**
 * Remove a folder from the selected set.
 */
bool CGlobal::remove_folder(std::string path)
{
    std::vector<std::string>::iterator it;

    /**
     * Find the folder.
     */
    it = std::find(m_selected_folders.begin(), m_selected_folders.end(), path);

    /**
     * If we found it remove it.
     */
    if (it != m_selected_folders.end())
    {
        m_selected_folders.erase(it);
        return true;
    }

    /**
     * Failed to find it.
     */
    return false;

}


/**
 * Get the value of the named string-variable.
 */
std::string * CGlobal::get_variable( std::string name )
{
    assert( ! name.empty() );

    std::string *value = m_variables[name];

#ifdef LUMAIL_DEBUG

    std::string dm = "Get variable named '";
    dm += name ;
    dm += "' value is '" ;

    if ( value != NULL )
        dm += *value;
    else
        dm += "NULL";

    dm+= "'";
    DEBUG_LOG( dm );

#endif /* LUMAIL_DEBUG */

    /**
     * Get the value.
     */
    return( value );
}


/**
 * Update the value of the named string variable.
 */
void CGlobal::set_variable( std::string name, std::string *value )
{
    assert( ! name.empty() );

    /**
     * Free the current value, if one is set.
     */
    std::string *current = m_variables[name];
    if ( current != NULL )
        delete( current );

    /**
     * Store new value.
     */
    m_variables[ name ] = value;

#ifdef LUMAIL_DEBUG
    std::string dm = "Set variable named '" ;
    dm += name ;
    dm += "' to value '";
    dm += *value;
    dm += "'";

    DEBUG_LOG( dm );
#endif
}


/**
 * Return our map of variables to the caller.
 */
std::unordered_map<std::string, std::string *> CGlobal::get_variables()
{
    return( m_variables );
}
