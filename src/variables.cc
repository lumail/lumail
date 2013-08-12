/**
 * variables.cc - Bindings for lua-visible variables
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
 *
 */


#include <stdio.h>
#include <algorithm>
#include <map>
#include <cstdlib>
#include <iostream>
#include <fstream>
#include <string.h>
#include <stdlib.h>
#include <pcrecpp.h>
#include <cursesw.h>
#include <unistd.h>
#include <dirent.h>

#include "debug.h"
#include "history.h"
#include "variables.h"
#include "global.h"
#include "file.h"


/**
 **  Helper functions.
 **/



/**
 * Get/Set the value of a string variable.
 *
 * This function is called from all our lua-exposed functions that
 * get/set a simple string value.
 */
int get_set_string_variable( lua_State *L, const char * name )
{
    const char *str = lua_tostring(L, -1);
    CGlobal *g = CGlobal::Instance();

    if (str != NULL)
    {

#ifdef LUMAIL_DEBUG

        std::string loggy;
        loggy = "Setting ";
        loggy += "'";
        loggy += name;
        loggy += "' to value '";
        loggy += str;
        loggy +=  "'";

        DEBUG_LOG(loggy);
#endif

        g->set_variable( name, new std::string(str));
    }

    std::string * s = g->get_variable(name);
    lua_pushstring(L, s->c_str());
    return( 1 );
}



/**
 ** Colour getters/setters.
 **/



/**
 * Colour of attachments in message header.
 */
int attachment_colour(lua_State *L)
{
    return( get_set_string_variable( L, "attachment_colour" ) );
}

/**
 * Colour of the message-body.
 */
int body_colour(lua_State *L)
{
    return( get_set_string_variable( L, "body_colour" ) );
}

/**
 * Colour of the message headers.
 */
int header_colour(lua_State *L)
{
    return( get_set_string_variable( L, "header_colour" ) );
}

/**
 * Colour of unread messages.
 */
int unread_message_colour(lua_State *L)
{
    return( get_set_string_variable( L, "unread_message_colour" ) );
}

/**
 * Colour of maildirs containing unread messages.
 */
int unread_maildir_colour(lua_State *L)
{
    return( get_set_string_variable( L, "unread_maildir_colour" ) );
}




/**
 ** General getters/setters.
 **/



/**
 * Get/set the completion characters we tokenize on.
 */
int completion_chars(lua_State *L)
{
    return( get_set_string_variable( L, "completion_chars" ) );
}

/**
 * Get, or set, the display-filter.
 */
int display_filter(lua_State * L)
{
    return( get_set_string_variable(L, "display_filter" ) );
}

/**
 * Get, or set, the editor
 */
int editor(lua_State * L)
{
    return( get_set_string_variable(L, "editor" ) );
}

/**
 * Get, or set, the default from address.
 */
int from(lua_State * L)
{
    return( get_set_string_variable(L, "from" ) );
}

/**
 * Get, or set, the global lumail mode.
 */
int global_mode(lua_State * L)
{
    return( get_set_string_variable(L, "global_mode" ) );
}

/**
 * Get, or set, the history persistance file.
 */
int history_file(lua_State *L )
{
    /**
     * This is valid only if we're setting the limit.
     */
    const char *str = lua_tostring(L, -1);

    int ret =  get_set_string_variable( L, "history_file" );

    /**
     * Load the history.
     */
    if ( str != NULL )
    {
        CHistory *history = CHistory::Instance();
        history->set_file( str );
    }

    return( ret );
}
/**
 * Get, or set, the index-format
 */
int index_format(lua_State * L)
{
    return( get_set_string_variable(L, "index_format" ) );
}

/**
 * Get, or set, the index limit.
 */
int index_limit(lua_State * L)
{
    /**
     * This is valid only if we're setting the limit.
     */
    const char *str = lua_tostring(L, -1);

    int ret =  get_set_string_variable( L, "index_limit" );

    /**
     * Update the messages and reset the current message offset
     * if we're *changing* the index-limit
     */
    if ( str != NULL )
    {
        CGlobal *global = CGlobal::Instance();
        global->update_messages();
        global->set_message_offset(0);
    }

    return ret;
}



/**
 * Get or set the mail filter.
 */
int mail_filter(lua_State * L)
{
    return( get_set_string_variable(L, "mail_filter" ) );
}

/**
 * The format-string for maildir-display.
 */
int maildir_format(lua_State *L )
{
    return( get_set_string_variable( L, "maildir_format" ) );
}

/**
 * Get, or set, the maildir limit.
 */
int maildir_limit(lua_State * L)
{
    /**
     * This is valid only if we're setting the limit.
     */
    const char *str = lua_tostring(L, -1);

    int ret =  get_set_string_variable( L, "maildir_limit" );


    /**
     * Update the maildirs.
     */
    if ( str != NULL )
    {
        CGlobal *global = CGlobal::Instance();
        global->update_maildirs();

        /**
         * Set the first maildir to be selected to avoid us highlighting
         * a maildir-entry which no longer exists under the cursor.
         */
        global->set_selected_folder(0);
    }

    return( ret );
}

/**
 * Get, or set, the maildir-prefix
 */
int maildir_prefix(lua_State * L)
{
    /**
     * If we're setting it, make sure the value is sane.
     */
    const char *str = lua_tostring(L, -1);
    if (str != NULL)
    {
        if ( !CFile::is_directory( str ) )
            return luaL_error(L, "The specified prefix is not a directory" );
    }

    return( get_set_string_variable(L, "maildir_prefix" ) );
}


/**
 * Get, or set, the sendmail path.
 */
int sendmail_path(lua_State * L)
{
    return( get_set_string_variable( L, "sendmail_path" ) );
}

/**
 * Get, or set, the sent-folder path.
 */
int sent_mail(lua_State * L)
{
    const char *str = lua_tostring(L, -1);
    if (str != NULL)
    {
        if ( !CMaildir::is_maildir( str ) )
            return luaL_error(L, "The specified sent_mail folder is not a Maildir" );
    }
    return( get_set_string_variable( L, "sent_mail" ) );
}


/**
 * Sorting choices.
 */
int sort(lua_State * L)
{
    /**
     * If updating then we refresh the messages.
     */
    const char *str = lua_tostring(L, -1);

    int ret = get_set_string_variable( L, "sort" );

    /**
     * Update the selected massages.
     */
    if ( str != NULL )
    {
        CGlobal *global = CGlobal::Instance();
        global->update_messages();

        /**
         * Reset the message offset because the current message
         * has probably changed.
         */
        global->set_message_offset(0);
    }
    return ret;

}


/**
 * Get the user's selected editor.
 */
std::string get_editor()
{
    /**
     * If this has been set use the editor
     */
    CGlobal *global = CGlobal::Instance();
    std::string *cmd  = global->get_variable("editor");
    if ( ( cmd != NULL ) && ( ! cmd->empty() ) )
        return( *cmd );

    /**
     * If there is an $EDITOR variable defined, use that.
     */
    std::string env = getenv( "EDITOR" );
    if ( !env.empty() )
        return( env );

    /**
     * Fall back to vim.
     */
    return( "vim" );

}

