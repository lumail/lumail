/**
 * $FILENAME - $TITLE
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


#include <cstdlib>

#include "lua.h"


/**
 * External functions implemented in *_lua.cc
 */
extern void InitConfig(lua_State * l);
extern void InitMaildir(lua_State * l);
extern void InitMessage(lua_State * l);
extern void InitMessagePart(lua_State * l);
extern void InitPanel(lua_State * l);
extern void InitScreen(lua_State * l);




/**
 * Get access to this singleton object.
 */
CLua * CLua::Instance()
{
    static CLua *instance = new CLua();
    return (instance);
}



/**
 * Constructor - This is private as this class is a singleton.
 */
CLua::CLua()
{
    /**
     * Create a new Lua object.
     */
    m_lua = luaL_newstate();


    /**
     * Register the libraries.
     */
    luaopen_base(m_lua);
    luaL_openlibs(m_lua);


    /**
     * Load our bindings.
     */
    InitConfig(m_lua);
    InitMaildir(m_lua);
    InitMessage(m_lua);
    InitMessagePart(m_lua);
    InitPanel(m_lua);
    InitScreen(m_lua);
}


/**
 * Destructor.
 */
CLua::~CLua()
{
    lua_close(m_lua);

}


/**
 * Load the specified Lua file, and evaluate it.
 *
 * Return true on success.  False on error.
 */
bool CLua::load_file(std::string filename)
{
    int erred = luaL_dofile(m_lua, filename.c_str());

    if (erred)
        return false;
    else
        return true;
}


/**
 * Evaluate the given string.
 *
 * Return true on success.  False on error.
 */
bool CLua::execute(std::string lua)
{
    if (luaL_dostring(m_lua, lua.c_str()))
        return false;
    else
        return true;
}


/**
 * Lookup a value in a nested-table - used for keyboard lookups.
 */
char * CLua::get_nested_table(std::string table, const char *key, const char *subkey)
{
    char *result = NULL;

    /**
     * Ensure the table exists - if it doesn't return NULL.
     */
    lua_getglobal(m_lua, table.c_str());

    if (lua_isnil(m_lua, -1))
    {
        return NULL;
    }

    /**
     * Get the sub-table.
     */
    lua_pushstring(m_lua, key);
    lua_gettable(m_lua, -2);

    if (!lua_istable(m_lua, -1))
        return result;

    /**
     * Get the value.
     */
    lua_pushstring(m_lua, subkey);
    lua_gettable(m_lua, -2);

    if (lua_isnil(m_lua, -1))
        return result;

    /**
     * If it worked, and is a string .. return it.
     */
    if (lua_isstring(m_lua, -1))
        result = (char *) lua_tostring(m_lua, -1);

    return result;
}
