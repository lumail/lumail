/**
 * lua.cc - Singleton interface to an embedded Lua interpreter.
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


#include <iostream>
#include <cstdlib>
#include <fstream>
#include <string.h>
#include <stdlib.h>


#include "bindings.h"
#include "debug.h"
#include "file.h"
#include "global.h"
#include "lua.h"
#include "version.h"


/**
 * Instance-handle.
 */
CLua *CLua::pinstance = NULL;



/**
 * This is a list of all the functions that we export to our
 * embedded lua-intepreter.
 *
 * The function is externally declared in our header so that we
 * can access it from elsewhere.
 *
 * (Specifically we want to make the function-names available to
 * our TAB-completion code.
 *
 */
struct CLuaMapping primitive_list[] =
{

/**
 * Basic primitives
 */
    {"abort", "Exit lumail with an error message.", (lua_CFunction) abort },
    {"clear", "Clear the screen.", (lua_CFunction) clear },
    {"alert", "SHow an alert which requires confirmation.", (lua_CFunction) alert },
    {"dump_stack", "Dump the Lua-stack for debugging purposes", (lua_CFunction) lua_dump_stack },
    {"log_message", "Add a message to the debug-log.", (lua_CFunction) log_message },
    {"exec", "Execute an external command.", (lua_CFunction) exec },
    {"exit", "Exit lumail.", (lua_CFunction) exit },
    {"mime_type", "Get the MIME-type for a file.", (lua_CFunction) mime_type },
    {"msg", "Write a message to the status-area.", (lua_CFunction) msg },
    {"refresh_display", "Refresh the display.", (lua_CFunction) refresh_display },
    {"screen_height", "Return the height of the screen in rows.", (lua_CFunction) screen_height },
    {"screen_width", "Return the width of the screen in columns.", (lua_CFunction) screen_width },
    {"sleep", "Pause execution for the given number of seconds.", (lua_CFunction) sleep },

/**
 * File/Path utilities
 */
    {"executable", "Is the given file executable?", (lua_CFunction) executable },
    {"file_exists", "Does the given file exist?", (lua_CFunction) file_exists },
    {"is_directory", "Is the given path a directory?", (lua_CFunction) is_directory },
    {"load_directory", "Load *.lua from beneath the given directory.  (Not recursively.)", (lua_CFunction) load_directory },

/**
 * Folder Selection
 */
    {"add_selected_folder", "help", (lua_CFunction) add_selected_folder },
    {"clear_selected_folders", "help", (lua_CFunction) clear_selected_folders },
    {"selected_folders", "help", (lua_CFunction) selected_folders },
    {"set_selected_folder", "help", (lua_CFunction) set_selected_folder },
    {"toggle_selected_folder", "help", (lua_CFunction) toggle_selected_folder },

/**
 * Get/Set variables
 */
    {"editor", "Query or update the editor to use.", (lua_CFunction) editor },
    {"from", "Query or update the from-address for outgoing mails.", (lua_CFunction) from },
    {"global_mode", "Query or update the global-mode.", (lua_CFunction) global_mode },
    {"index_format", "Query or update the index-format string.", (lua_CFunction) index_format },
    {"index_limit", "Query or update the index-limit string.", (lua_CFunction) index_limit },
    {"maildir_format", "Query or update the maildir-format string.", (lua_CFunction) maildir_format },
    {"maildir_limit", "Query or update the maildir-limit string.", (lua_CFunction) maildir_limit },
    {"maildir_prefix", "Query or update the root of the Maildir hierarchy.", (lua_CFunction) maildir_prefix },
    {"message_filter", "Query or update the filter to apply to messages being viewed.", (lua_CFunction) message_filter },
    {"sendmail_path", "Query or update the sendmail-path, used for sending mails.", (lua_CFunction) sendmail_path },
    {"sent_mail", "Query or update the Maildir location to send outgoing mails to.", (lua_CFunction) sent_mail },
    {"sort", "Query or update the sorting string for index-mode.", (lua_CFunction) sort },
    {"get_variables", "Retrieve all known-variables and their values.", (lua_CFunction) get_variables },

/**
 * Colour getters/setters
 */
    {"attachment_colour", "Get/Set the colour to use for drawing attachments.", (lua_CFunction) attachment_colour },
    {"body_colour", "Get/Set the colour to use for drawing message-bodies.", (lua_CFunction) body_colour },
    {"header_colour", "Get/Set the colour to use for drawing message-headers.", (lua_CFunction) header_colour },
    {"unread_maildir_colour", "Get/Set the colour to use for drawing Maildirs which contain unread messages.", (lua_CFunction) unread_maildir_colour },
    {"unread_message_colour", "Get/Set the colour to use for drawing unread messages.", (lua_CFunction) unread_message_colour },

/**
 * Index functions
 */
    {"jump_index_to", "help", (lua_CFunction) jump_index_to },
    {"scroll_index_down", "help", (lua_CFunction) scroll_index_down },
    {"scroll_index_up", "help", (lua_CFunction) scroll_index_up },
    {"scroll_index_to", "help", (lua_CFunction) scroll_index_to },

/**
 * Message-Related functions
 */
    {"compose", "Compose a new outgoing email.", (lua_CFunction) compose },
    {"count_messages", "Count the messages in the currently selected Maildir(s).", (lua_CFunction) count_messages },
    {"count_lines", "Count the number of lines in the message body", (lua_CFunction) count_lines},
    {"current_message", "Retrieve the path to the current message.", (lua_CFunction) current_message },
    {"delete", "Delete the current message.", (lua_CFunction) delete_message },
    {"header", "Retrieve the value of the given header from the current message.", (lua_CFunction) header },
    {"is_new", "Is the current message new/unread?", (lua_CFunction) is_new },
    {"mark_new", "Mark the current message as new/unread.", (lua_CFunction) mark_new },
    {"mark_read", "Mark the current message as old/read.", (lua_CFunction) mark_read },
    {"reply", "Reply to the current message.", (lua_CFunction) reply },
    {"save", "Save the current message in a new location, and delete it.", (lua_CFunction) save_message },
    {"save_message", "Save the current message in a new location, and delete it.", (lua_CFunction) save_message },
    {"scroll_message_down", "Scroll the current message down.", (lua_CFunction) scroll_message_down },
    {"scroll_message_up", "Scroll the current message up.", (lua_CFunction) scroll_message_up },
    {"scroll_message_to", "Scroll the current message to the given offset.", (lua_CFunction) scroll_message_to },
    {"send_email", "Send an email, via Lua.", (lua_CFunction) send_email },

/**
 * Maildirs
 */
    {"count_maildirs", "help", (lua_CFunction) count_maildirs },
    {"current_maildir", "help", (lua_CFunction) current_maildir },
    {"current_maildirs", "help", (lua_CFunction) current_maildirs },
    {"jump_maildir_to", "help", (lua_CFunction) jump_maildir_to },
    {"maildirs_matching", "help", (lua_CFunction) maildirs_matching },
    {"scroll_maildir_down", "help", (lua_CFunction) scroll_maildir_down },
    {"scroll_maildir_to", "help", (lua_CFunction) scroll_maildir_to },
    {"scroll_maildir_up", "help", (lua_CFunction) scroll_maildir_up },
    {"select_maildir", "help", (lua_CFunction) select_maildir },


/**
 * prompts.
 */
    {"prompt", "Prompt for input.", (lua_CFunction) prompt },
    {"prompt_chars", "Prompt for input, until one of a given number of characters is entered.", (lua_CFunction) prompt_chars },
    {"prompt_maildir", "Prompt for an (existing) Maildir.", (lua_CFunction) prompt_maildir },
    {"prompt_yn", "Prompt for a yes/no answer.", (lua_CFunction) prompt_yn },

/**
 * Attachments
 */
    {"attachments", "Retrieve the list of attachments in the current message.", (lua_CFunction) attachments },
    {"count_attachments", "Count the number of attachments the current message contains.", (lua_CFunction) count_attachments },
    {"save_attachment", "Save the given attachment to disk, from the current message.", (lua_CFunction) save_attachment },


};


/**
 * The size of our primitives table
 */
int primitive_count = (sizeof(primitive_list)/sizeof(struct CLuaMapping));



/**
 * Get access to our LUA intepreter.
 */
CLua *CLua::Instance()
{
    if (!pinstance)
        pinstance = new CLua;

    return pinstance;
}



/**
 * Constructor - This is private as this class is a singleton.
 */
CLua::CLua()
{
    /**
     * Create a new LUA object.
     */
    m_lua = luaL_newstate();

    /**
     * Register the libraries.
     */
    luaopen_base(m_lua);
    luaL_openlibs(m_lua);

    /**
     * Now register our primitives.
     */
    for(int i = 0; i < primitive_count; i++ )
    {
        lua_pushcfunction(m_lua, primitive_list[i].func);
        lua_setglobal(m_lua, primitive_list[i].name);
    }


    /**
     * Set a global variable into the Lua environment.
     */
    lua_pushstring(m_lua, LUMAIL_VERSION );
    lua_setglobal(m_lua, "VERSION" );

}


/**
 * Load the specified lua file, and evaluate it.
 */
bool CLua::load_file(std::string filename)
{
    if (CFile::exists( filename ) )
    {
        if (luaL_loadfile(m_lua, filename.c_str())
            || lua_pcall(m_lua, 0, 0, 0))
        {
            fprintf(stderr, "cannot run configuration file: %s",
                    lua_tostring(m_lua, -1));
            exit(1);
        }
        return true;
    }
    return false;
}


/**
 * Evaluate the given string.
 */
void CLua::execute(std::string lua)
{
    if ( luaL_dostring(m_lua, lua.c_str()))
    {

#ifdef LUMAIL_DEBUG
        std::string dm = "CLua::execute(\"";
        dm += lua;
        dm += "\"); -> ";

        const char *err = lua_tostring(m_lua, -1);
        dm += err;
        DEBUG_LOG( dm );
#endif
    }
}

/**
 * Call a single Lua function, passing no arguments and ignoring the return code.
 */
bool CLua::call_function(std::string name)
{
    lua_getglobal(m_lua, name.c_str());
    if (lua_isfunction(m_lua, -1))
    {
        lua_pcall(m_lua, 0, 0, 0);
        return true;
    }
    else
        return false;
}


/**
 * get the value from a nested table.
 */
char *CLua::get_nested_table( std::string table, std::string key, std::string subkey )
{
    char *result = NULL;

    /**
     * Ensure the table exists.
     */
    lua_getglobal(m_lua, table.c_str() );
    if (lua_isnil (m_lua, -1 ) )
        return result;

    /**
     * Get the sub-table.
     */
    lua_pushstring(m_lua, key.c_str() );
    lua_gettable(m_lua, -2);
    if (! lua_istable(m_lua, -1 ) )
        return result;

    /**
     * Get the value.
     */
    lua_pushstring(m_lua, subkey.c_str());
    lua_gettable(m_lua, -2);
    if (lua_isnil (m_lua, -1 ) )
        return result;

    /**
     * If it worked, and is a string .. return it.
     */
    if (lua_isstring(m_lua, -1))
        result = (char *)lua_tostring(m_lua, -1);
    return result;
}


/**
 * Lookup the binding for the named keystroke in our keymap(s).
 *
 * If the result is a string then execute it as a function.
 */
bool CLua::on_keypress(const char *keypress)
{
    /**
     * The result of the lookup.
     */
    char *result = NULL;

    /**
     * Get the current global-mode.
     */
    CGlobal *global   = CGlobal::Instance();
    std::string *mode = global->get_variable("global_mode");

    /**
     * Lookup the keypress in the current-mode-keymap.
     */
    result = get_nested_table( "keymap", mode->c_str(), keypress );


    /**
     * If that failed then lookup the global keymap.
     *
     * This order ensures you can have a "global" keymap, overridden in just one mode.
     */
    if ( result == NULL )
        result = get_nested_table( "keymap", "global", keypress );

    /**
     * If one/other of these lookups resulted in success then we're golden.
     */
    if ( result != NULL )
        execute(result);

    /**
     * We succeeded if the result wasn't NULL.
     */
    return( result != NULL );
}

/**
 * Invoke the lua-defined on_key() callback.
 */
bool CLua::on_key(const char *key )
{
    /**
     * Get the "on_key()" function, and see if it exists.
     */
    lua_getglobal( m_lua, "on_key" );
    if(!lua_isfunction(m_lua,-1))
        return false;


    /**
     * Push the key and call it.
     */
    lua_pushstring( m_lua, key );
    lua_pcall(m_lua, 1, 1 , 0 ) ;

    /**
     * Get the return value.
     */
    int ret = lua_tointeger(m_lua,-1);
    lua_pop(m_lua,1);
    if ( ret )
        return true;

    /**
     * Fail
     */
    return false;
}


std::vector<std::string> CLua::on_complete()
{
    std::vector<std::string> results;

    /**
     * If the global isn't defined then we return
     * an empty set.
     */
    lua_getglobal( m_lua, "on_complete" );
    if(!lua_isfunction(m_lua,-1))
        return results;

    /**
     * Call it.
     */
    lua_pcall(m_lua, 0, 1 , 0 ) ;


    if (lua_type(m_lua, -1)!=LUA_TTABLE)
    {
        lua_pop(m_lua, 1);
        return results;
    }

    lua_pushnil(m_lua);

    while (lua_next(m_lua, -2))
    {
        const char *d  = lua_tostring(m_lua, -1);
        results.push_back( d );
        lua_pop( m_lua , 1);
    }
    return( results );
}


/**
 * convert a table to an array of strings.
 */
std::vector<std::string> CLua::table_to_array( std::string name )
{
    std::vector<std::string> results;

#ifdef LUMAIL_DEBUG
    std::string ds = "table_to_array(";
    ds += name;
    ds += ") - start";
    DEBUG_LOG( ds );
#endif

    /**
     * Ensure we have a table.
     */
    lua_getglobal(m_lua, name.c_str() );
    if (lua_type(m_lua, -1)!=LUA_TTABLE)
    {
        lua_pop(m_lua, 1);
        return results;
    }

    lua_pushnil(m_lua);


    while (lua_next(m_lua, -2))
    {
        const char *d  = lua_tostring(m_lua, -1);

#ifdef LUMAIL_DEBUG
        std::string dm = "\tFound:";
        dm += d;
        DEBUG_LOG( dm );
#endif

        results.push_back( d );
        lua_pop( m_lua , 1);
    }



#ifdef LUMAIL_DEBUG
    std::string de = "table_to_array(";
    de += name;
    de += ") - end";
    DEBUG_LOG( de );
#endif


    lua_pop( m_lua , 1);
    return( results );
}


bool CLua::get_bool( std::string  name )
{
    bool ret = false;
    lua_getglobal(m_lua, name.c_str() );
    if (lua_type(m_lua, -1)!=LUA_TBOOLEAN)
    {
        lua_pop(m_lua, 1);
        return ret;
    }
    ret = lua_toboolean(m_lua,-1);
    lua_pop(m_lua,1);
    return ret;

}


/**
 * Dump the stack contents - only in debug-builds.
 */
void CLua::dump_stack()
{
#ifdef LUMAIL_DEBUG

    DEBUG_LOG( "dump_stack() - start" );

    /**
     * Top of the stack.
     */
    int top = lua_gettop(m_lua);

    /* Repeat for each level */
    for (int i = 1; i <= top; i++)
    {
        char buf[1024]={'\0'};

        int t = lua_type(m_lua, i);
        switch (t)
        {
        case LUA_TSTRING:
            snprintf(buf, sizeof(buf)-1,"`%s'", lua_tostring(m_lua, i));
            break;

        case LUA_TBOOLEAN:
            snprintf(buf, sizeof(buf)-1,lua_toboolean(m_lua, i) ? "true" : "false");
            break;

        case LUA_TNUMBER:
            snprintf(buf, sizeof(buf)-1,"%g", lua_tonumber(m_lua, i));
            break;

        default:
            snprintf(buf, sizeof(buf)-1,"%s", lua_typename(m_lua, t));
            break;

        }

        DEBUG_LOG( buf );
    }

    DEBUG_LOG( "dump_stack() - end" );
#endif

}
