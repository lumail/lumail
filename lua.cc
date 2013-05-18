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
#include <sys/stat.h>
#include <sys/types.h>
#include <string.h>
#include <stdlib.h>


#include "lua.h"
#include "bindings.h"
#include "global.h"
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
     * Version number
     */
    set_global("VERSION", LUMAIL_VERSION);

    /**
     * Register our primitives - the basic ones.
     */
    lua_register(m_lua, "clear", clear);
    lua_register(m_lua, "exec", exec);
    lua_register(m_lua, "exit", exit);
    lua_register(m_lua, "msg", msg);
    lua_register(m_lua, "prompt", prompt);
    lua_register(m_lua, "sleep", sleep);

    /**
     * Get/Set the global mode.
     */
    lua_register(m_lua, "global_mode", global_mode);

    /**
     * Get/Set the maildir prefix.
     */
    lua_register(m_lua, "maildir_prefix", maildir_prefix);

    /**
     * Get/Set the index format.
     */
    lua_register(m_lua, "index_format", index_format);
    // TODO: message_headers( "To", "From", "Cc", etc.

    /**
     * Scroll mailboxes up/down/to a pattern.
     */
    lua_register(m_lua, "scroll_maildir_down", scroll_maildir_down);
    lua_register(m_lua, "scroll_maildir_up", scroll_maildir_up);
    lua_register(m_lua, "scroll_maildir_to", scroll_maildir_to);
    lua_register(m_lua, "jump_maildir_to", jump_maildir_to);

    /**
     * Scroll index up/down/to a pattern.
     */
    lua_register(m_lua, "scroll_index_down", scroll_index_down);
    lua_register(m_lua, "scroll_index_up", scroll_index_up);
    lua_register(m_lua, "scroll_index_to", scroll_index_to);
    lua_register(m_lua, "jump_index_to", jump_index_to );

    /**
     * Get the current maildir.
     */
    lua_register(m_lua, "current_maildir", current_maildir);
    lua_register(m_lua, "count_maildirs", count_maildirs );

    /**
     * Get the current message
     */
    lua_register(m_lua, "current_message", current_message);

    /**
     * Message status manipulation.
     */
    lua_register(m_lua, "is_new", is_new);
    lua_register(m_lua, "mark_new", mark_new);
    lua_register(m_lua, "mark_read", mark_read);
    lua_register(m_lua, "delete", delete_message);

    /**
     * Folder selection.
     */
    lua_register(m_lua, "selected_folders", selected_folders);
    lua_register(m_lua, "clear_selected_folders", clear_selected_folders);
    lua_register(m_lua, "add_selected_folder", add_selected_folder);
    lua_register(m_lua, "toggle_selected_folder", toggle_selected_folder);
    lua_register(m_lua, "set_selected_folder", set_selected_folder);
    lua_register(m_lua, "count_messages", count_messages );

    /**
     * Get/Set the maildir-limit & index-limit.
     */
    lua_register(m_lua, "maildir_limit", maildir_limit);
    lua_register(m_lua, "index_limit", index_limit);

    /**
     * Compose a new mail.
     */
    lua_register(m_lua, "compose", compose);
    // TODO: reply()

    /**
     * Get screen dimensions.
     */
    lua_register(m_lua, "screen_width", screen_width);
    lua_register(m_lua, "screen_height", screen_height);


    /**
     * Set the From: address for the user.
     */
    lua_register(m_lua, "from", from);
    //TODO: sendmail_path
}


/**
 * Load the specified lua file, and evaluate it.
 */
void CLua::load_file(std::string filename)
{
    struct stat sb;

    std::cout << "Loading file " << filename << std::endl;
    if ((stat(filename.c_str(), &sb) == 0))
    {
	if (luaL_loadfile(m_lua, filename.c_str())
	    || lua_pcall(m_lua, 0, 0, 0))
        {
	    fprintf(stderr, "cannot run configuration file: %s",
		    lua_tostring(m_lua, -1));
	    exit(1);
	}
    }
}


/**
 * Evaluate the given string.
 */
void CLua::execute(std::string lua)
{
    luaL_dostring(m_lua, lua.c_str());
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
 * Set a global variable into the Lua environment.
 */
void CLua::set_global(std::string name, std::string value)
{
    lua_pushstring(m_lua, value.c_str());
    lua_setglobal(m_lua, name.c_str());
}

/**
 * Get a global variable value from the Lua environment.
 */
std::string * CLua::get_global(std::string name)
{
    std::string * result = NULL;

    lua_getglobal(m_lua, name.c_str());
    if (!lua_isnil(m_lua, -1))
	result = new std::string(lua_tostring(m_lua, -1));

    lua_pop(m_lua, 1);

    return result;
}


/**
 * Execute a function from the global keymap.
 */
bool CLua::on_keypress(char key)
{
    char keypress[2];
    keypress[0] = key;
    keypress[1] = '\0';

    /**
     * The result of the lookup.
     */
    char *result = NULL;

    /**
     * Get the current global-mode.
     */
    CGlobal *g = CGlobal::Instance();
    std::string *mode = g->get_mode();

    /**
     * Lookup the keypress in the global-keymap.
     */
    lua_getglobal(m_lua, "keymap");
    lua_pushstring(m_lua, "global" );
    lua_gettable(m_lua, -2);
    if ( lua_istable(m_lua, -1 ) )
    {
        lua_pushstring(m_lua, keypress);
        lua_gettable(m_lua, -2);

        if (lua_isstring(m_lua, -1))
            result = (char *)lua_tostring(m_lua, -1);
    }


    /**
     * Lookup the keypress in the per-mode keymap.
     */
    lua_getglobal(m_lua, "keymap");
    lua_pushstring(m_lua, mode->c_str() );
    lua_gettable(m_lua, -2);
    if ( lua_istable(m_lua, -1 ) )
    {
        lua_pushstring(m_lua, keypress);
        lua_gettable(m_lua, -2);
        if (lua_isstring(m_lua, -1))
            result = (char *)lua_tostring(m_lua, -1);

    }

    /**
     * If one/other of these resulted in success then we're golden.
     */
    if ( result != NULL )
    {
        execute(result);
        return true;
    }
    return false;
}


