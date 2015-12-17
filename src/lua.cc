/*
 * lua.cc - Wrapper for the lua interpretter.
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
#include <iostream>
#include <string.h>

#include "config.h"
#include "lua.h"
#include "screen.h"


/*
 * External functions implemented in *_lua.cc
 */
extern void InitConfig(lua_State * l);
extern void InitDirectory(lua_State * l);
extern void InitFile(lua_State * l);
extern void InitGlobalState(lua_State * l);
extern void InitLogfile(lua_State * l);
extern void InitMaildir(lua_State * l);
extern void InitMessage(lua_State * l);
extern void InitMIME(lua_State * l);
extern void InitMessagePart(lua_State * l);
extern void InitNet(lua_State * l);
extern void InitPanel(lua_State * l);
extern void InitRegexp(lua_State * l);
extern void InitScreen(lua_State * l);



/*
 * Populate "args"
 */
void CLua::set_args(char *argv[], int argc)
{
    lua_newtable(m_lua);

    for (int i = 0; i < argc; i++)
    {
        lua_pushinteger(m_lua, i + 1);
        lua_pushstring(m_lua, argv[i]);
        lua_settable(m_lua, -3);
    }

    lua_setglobal(m_lua, "ARGS");
}



/*
 * Constructor - This is private as this class is a singleton.
 */
CLua::CLua() : Observer(CConfig::instance())
{
    /*
     * Create a new Lua object.
     */
    m_lua = luaL_newstate();


    /*
     * Register the libraries.
     */
    luaopen_base(m_lua);
    luaL_openlibs(m_lua);


    /*
     * Load our bindings.
     */
    InitConfig(m_lua);
    InitDirectory(m_lua);
    InitFile(m_lua);
    InitGlobalState(m_lua);
    InitLogfile(m_lua);
    InitMaildir(m_lua);
    InitMessage(m_lua);
    InitMessagePart(m_lua);
    InitNet(m_lua);
    InitPanel(m_lua);
    InitMIME(m_lua);
    InitRegexp(m_lua);
    InitScreen(m_lua);
}


/*
 * Destructor.
 */
CLua::~CLua()
{
    lua_close(m_lua);

}


/*
 * Load the specified Lua file, and evaluate it.
 *
 * If there is an error loading the file then the program will abort,
 * and the error message will be shown to the user.
 */
void CLua::load_file(std::string filename)
{
    int erred = luaL_dofile(m_lua, filename.c_str());

    if (erred)
    {
        char *err = NULL;

        if (lua_isstring(m_lua, -1))
            err = (char *)strdup(lua_tostring(m_lua, -1));

        /*
         * Abort - showing the error
         */
        CScreen *screen = CScreen::instance();
        screen->teardown();
        std::cerr << "ERROR " << err << std::endl;
        exit(2);

        /*
         * Avoid a leak.
         */
        free(err);
    }

}


/*
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


/*
 * Lookup a value in a nested-table - used for keyboard lookups.
 */
char * CLua::get_nested_table(std::string table, const char *key, const char *subkey)
{
    char *result = NULL;

    /*
     * Ensure the table exists - if it doesn't return NULL.
     */
    lua_getglobal(m_lua, table.c_str());

    if (lua_isnil(m_lua, -1))
    {
        return NULL;
    }

    /*
     * Get the sub-table.
     */
    lua_pushstring(m_lua, key);
    lua_gettable(m_lua, -2);

    if (!lua_istable(m_lua, -1))
        return result;

    /*
     * Get the value.
     */
    lua_pushstring(m_lua, subkey);
    lua_gettable(m_lua, -2);

    if (lua_isnil(m_lua, -1))
        return result;

    /*
     * If it worked, and is a string .. return it.
     */
    if (lua_isstring(m_lua, -1))
        result = (char *) lua_tostring(m_lua, -1);

    return result;
}


/*
 * Call `on_complete` to complete a string.
 */
std::vector<std::string> CLua::get_completions(std::string token)
{
    std::vector<std::string> result;

    lua_getglobal(m_lua, "on_complete");
    lua_pushstring(m_lua, token.c_str());

    if (lua_pcall(m_lua, 1, 1, 0) != 0)
    {
        if (lua_isstring(m_lua, -1))
        {
            /*
             * The error message will be on the stack..
             */
            char *err = strdup(lua_tostring(m_lua, -1));
            lua_pop(m_lua, 1);

            /*
             * Call the function - if it exists.
             */
            lua_getglobal(m_lua, "on_error");

            if (!lua_isnil(m_lua, -1))
            {
                lua_pushstring(m_lua, err);

                if (lua_pcall(m_lua, 1, 0, 0) != 0)
                {
                    /*
                     * Error invoking our error handler - ignore it.
                     */
                    lua_pop(m_lua, 1);
                }
            }

            /*
             * Avoid a leak.
             */
            free(err);
        }
        else
            lua_pop(m_lua, 1);

        /*
         * Now return the result.
         */
        return result;
    }

    lua_pushnil(m_lua);

    while (lua_next(m_lua, -2))
    {
        const char *entry = lua_tostring(m_lua, -1);
        result.push_back(entry);
        lua_pop(m_lua, 1);

    }

    return (result);
}


/*
 * This method is called when a configuration key changes,
 * via our observer implementation.
 */
void CLua::update(std::string key_name)
{
    /*
     * If there is a Config:key_changed() function, then call it.
     */
    lua_getglobal(m_lua, "Config");
    lua_getfield(m_lua, -1, "key_changed");

    if (lua_isnil(m_lua, -1))
        return;

    /*
     * Call the function.
     */
    lua_pushstring(m_lua, key_name.c_str());

    if (lua_pcall(m_lua, 1, 0, 0) != 0)
    {
        if (lua_isstring(m_lua, -1))
        {
            /*
             * The error message will be on the stack..
             */
            char *err = strdup(lua_tostring(m_lua, -1));
            lua_pop(m_lua, 1);

            /*
             * Call the function - if it exists.
             */
            lua_getglobal(m_lua, "on_error");

            if (!lua_isnil(m_lua, -1))
            {
                lua_pushstring(m_lua, err);

                if (lua_pcall(m_lua, 1, 0, 0) != 0)
                {
                    /*
                     * Error invoking our error handler - ignore it.
                     */
                    lua_pop(m_lua, 1);
                }
            }

            /*
             * Avoid a leak.
             */
            free(err);
        }
        else
            lua_pop(m_lua, 1);
    }
}

/*
 * Call a Lua function which will return a table of text.
 */
std::vector<std::string> CLua::function2table(std::string function)
{
    std::vector<std::string> result;

    /*
     * Get the fuction - if it doesn't exist we're done.
     */
    lua_getglobal(m_lua, function.c_str());

    if (lua_isnil(m_lua, -1))
        return (result);


    /*
     * Call the function - and handle any error.
     */
    if (lua_pcall(m_lua, 0, 1, 0) != 0)
    {
        if (lua_isstring(m_lua, -1))
        {
            /*
             * The error message will be on the stack..
             */
            char *err = strdup(lua_tostring(m_lua, -1));
            lua_pop(m_lua, 1);

            /*
             * Call the function - if it exists.
             */
            lua_getglobal(m_lua, "on_error");

            if (!lua_isnil(m_lua, -1))
            {
                lua_pushstring(m_lua, err);

                if (lua_pcall(m_lua, 1, 0, 0) != 0)
                {
                    /*
                     * Error invoking our error handler - ignore it.
                     */
                    lua_pop(m_lua, 1);
                }
            }

            /*
             * Avoid a leak.
             */
            free(err);
        }
        else
            lua_pop(m_lua, 1);

        return (result);
    }

    /*
     * Now get the table we expected.
     */
    if (lua_istable(m_lua, 1))
    {
        lua_pushnil(m_lua);

        while (lua_next(m_lua, -2))
        {
            const char *entry = lua_tostring(m_lua, -1);
            result.push_back(entry);
            lua_pop(m_lua, 1);
        }
    }

    return (result);
}
