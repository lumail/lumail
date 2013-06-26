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
     * Create LUA object.
     */
    m_lua = lua_open();

    luaopen_base(m_lua);
    luaL_openlibs(m_lua);

    /**
     * Set a global variable into the Lua environment.
     */
    lua_pushstring(m_lua, LUMAIL_VERSION );
    lua_setglobal(m_lua, "VERSION" );

    /**
     * Register our primitives - the basic ones.
     */
    lua_register(m_lua, "abort", abort);
    lua_register(m_lua, "clear", clear);
    lua_register(m_lua, "exec", exec);
    lua_register(m_lua, "exit", exit);
    lua_register(m_lua, "mime_type", mime_type);
    lua_register(m_lua, "msg", msg);
    lua_register(m_lua, "prompt", prompt);
    lua_register(m_lua, "prompt_chars", prompt_chars);
    lua_register(m_lua, "prompt_maildir", prompt_maildir);
    lua_register(m_lua, "prompt_yn", prompt_yn);
    lua_register(m_lua, "refresh_display", refresh_display);
    lua_register(m_lua, "sleep", sleep);

    /**
     * Get/Set various strings.
     */
    lua_register(m_lua, "editor", editor);
    lua_register(m_lua, "from", from);
    lua_register(m_lua, "global_mode", global_mode);
    lua_register(m_lua, "index_format", index_format);
    lua_register(m_lua, "index_limit", index_limit);
    lua_register(m_lua, "maildir_format", maildir_format);
    lua_register(m_lua, "maildir_limit", maildir_limit);
    lua_register(m_lua, "maildir_prefix", maildir_prefix);
    lua_register(m_lua, "message_filter", message_filter);
    lua_register(m_lua, "sendmail_path", sendmail_path );
    lua_register(m_lua, "sent_mail", sent_mail );


    /**
     * Scroll mailboxes up/down/to a pattern.
     */
    lua_register(m_lua, "jump_maildir_to", jump_maildir_to);
    lua_register(m_lua, "scroll_maildir_down", scroll_maildir_down);
    lua_register(m_lua, "scroll_maildir_to", scroll_maildir_to);
    lua_register(m_lua, "scroll_maildir_up", scroll_maildir_up);

    /**
     * Scroll index up/down/to a pattern.
     */
    lua_register(m_lua, "jump_index_to", jump_index_to );
    lua_register(m_lua, "scroll_index_down", scroll_index_down);
    lua_register(m_lua, "scroll_index_to", scroll_index_to);
    lua_register(m_lua, "scroll_index_up", scroll_index_up);

    /**
     * Scroll message up/down.
     */
    lua_register(m_lua, "scroll_message_down", scroll_message_down);
    lua_register(m_lua, "scroll_message_up", scroll_message_up);


    /**
     * Maildir-related functions.
     */
    lua_register(m_lua, "count_maildirs", count_maildirs );
    lua_register(m_lua, "current_maildir", current_maildir);
    lua_register(m_lua, "current_maildirs", current_maildirs);
    lua_register(m_lua, "maildirs_matching", maildirs_matching );
    lua_register(m_lua, "select_maildir", select_maildir );


    /**
     * Message-related functions.
     */
    lua_register(m_lua, "count_messages", count_messages );
    lua_register(m_lua, "current_message", current_message);
    lua_register(m_lua, "delete", delete_message);
    lua_register(m_lua, "header", header);
    lua_register(m_lua, "is_new", is_new);
    lua_register(m_lua, "mark_new", mark_new);
    lua_register(m_lua, "mark_read", mark_read);

    /**
     * save is new, save_message is depreciated.
     */
    lua_register(m_lua, "save", save_message);
    lua_register(m_lua, "save_message", save_message);

    /**
     * Folder selection.
     */
    lua_register(m_lua, "add_selected_folder", add_selected_folder);
    lua_register(m_lua, "clear_selected_folders", clear_selected_folders);
    lua_register(m_lua, "selected_folders", selected_folders);
    lua_register(m_lua, "set_selected_folder", set_selected_folder);
    lua_register(m_lua, "toggle_selected_folder", toggle_selected_folder);

    /**
     * Compose a new mail/reply to an existing one.
     */
    lua_register(m_lua, "compose", compose);
    lua_register(m_lua, "reply", reply);
    lua_register(m_lua, "send_email", send_email );


    /**
     * Get screen dimensions.
     */
    lua_register(m_lua, "screen_width", screen_width);
    lua_register(m_lua, "screen_height", screen_height);


    /**
     * File/Utility handlers. Useful for writing portable configuration files.
     */
    lua_register(m_lua, "executable", executable);
    lua_register(m_lua, "file_exists", file_exists);
    lua_register(m_lua, "is_directory", is_directory);

    /**
     * Variables.
     */
    lua_register(m_lua, "get_variables", get_variables );
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
    if (lua_isnil (m_lua, -1 ) ) {
        return result;
    }

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
 * Call the lua-function on_key()
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

/**
 * convert a table to an array of strings.
 */
std::vector<std::string> CLua::table_to_array( std::string name )
{
    std::vector<std::string> results;

    /**
     * Ensure we have a table.
     */
    lua_getglobal(m_lua, name.c_str() );
    if (lua_type(m_lua, -1)!=LUA_TTABLE) {
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

    lua_pop( m_lua , 1); // pop nil
    return( results );
}


/**
 * Dump the stack contents - only in debug-builds.
 */
void CLua::dump_stack()
{
#ifdef LUMAIL_DEBUG

    /**
     * Top of the stack.
     */
    int top = lua_gettop(m_lua);

    /* Repeat for each level */
    for (int i = 1; i <= top; i++)
    {
        int t = lua_type(m_lua, i);
        switch (t)
        {
        case LUA_TSTRING:
            printf("`%s'", lua_tostring(m_lua, i));
            break;

        case LUA_TBOOLEAN:
            printf(lua_toboolean(m_lua, i) ? "true" : "false");
            break;

        case LUA_TNUMBER:
            printf("%g", lua_tonumber(m_lua, i));
            break;

        default:
            printf("%s", lua_typename(m_lua, t));
            break;

        }
        printf("  ");
    }
    printf("\n");
#endif

}
