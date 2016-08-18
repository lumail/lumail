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
#include "logfile.h"

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
 * For debug-purposes the nesting starts at zero.
 */
int CLuaLog::m_nest = 0;


/*
 * Populate "args"
 */
void CLua::set_args(char *argv[], int argc)
{
    CLuaLog("set_args()");

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
    CLuaLog("load_file(" + filename + ")");

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

void CLua::on_error(std::string msg)
{
    CLuaLog("on_error(" + msg + ")");

    /*
     * Call the function - if it exists.
     */
    lua_getglobal(m_lua, "on_error");

    if (!lua_isnil(m_lua, -1))
    {
        lua_pushstring(m_lua, msg.c_str());

        if (lua_pcall(m_lua, 1, 0, 0) != 0)
        {
            /*
             * Error invoking our error handler - ignore it.
             */
            lua_pop(m_lua, 1);
        }
    }
}

/*
 * Evaluate the given string.
 *
 * Return true on success.  False on error.
 */
bool CLua::execute(std::string lua)
{
    CLuaLog("execute(" + lua + ")");

    int result = luaL_loadstring(m_lua, lua.c_str());

    if (result != 0)
    {
        std::string err = lua_tostring(m_lua, -1);
        on_error(err);
        return false;
    }

    /* Since luaL_loadstring succeeded, the compiled function is on top of
     * the stack.
     */
    result = lua_pcall(m_lua, 0, LUA_MULTRET, 0);

    if (result == 0)
    {
        return true;
    }
    else
    {
        std::string err = lua_tostring(m_lua, -1);
        on_error(err);
        return false;
    }
}



/*
 * Does the specified function exist (in lua)?
 */
bool CLua::function_exists(std::string function)
{
    CLuaLog("function_exists(" + function + ")");

    /*
     * Get the function.
     */
    lua_getglobal(m_lua, function.c_str());

    /*
     * If the result was nil then it doesn't exist.
     */
    if (lua_isnil(m_lua, -1))
        return false;

    /*
     * If the type of the global is not a function then
     * it doesn't exist.
     */
    if (! lua_isfunction(m_lua, -1))
        return false;


    return true;
}


/*
 * Get the known-bindings of the keymap in the given mode.
 *
 * This just returns the names of the keys, rather than their values.
 */
std::vector<std::string> CLua::bindings(std::string mode)
{
    CLuaLog("bindings(" + mode + ")");

    std::vector<std::string> result;

    /*
     * Ensure the table exists - if it doesn't return NULL.
     */
    lua_getglobal(m_lua, "keymap");

    if (lua_isnil(m_lua, -1))
        return (result);

    /*
     * Get the sub-table.
     */
    lua_pushstring(m_lua, mode.c_str());
    lua_gettable(m_lua, -2);

    if (!lua_istable(m_lua, -1))
        return (result);


    /*
     * Now iterate over the keys.
     */
    lua_pushnil(m_lua);

    while (lua_next(m_lua, -2))
    {
        const char *entry = lua_tostring(m_lua, -2);
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
    CLuaLog("update(" + key_name + ")");

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

            on_error(err);

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
 *
 * If the argument is not equal to "" then it will be given as the
 * argument to the specified Lua function.
 */
std::vector<std::string> CLua::function2table(std::string function, std::string argument)
{
    CLuaLog("function2table(" + function + "," + argument + ")");

    std::vector<std::string> result;

    /*
     * Get the function - if it doesn't exist we're done.
     */
    lua_getglobal(m_lua, function.c_str());

    if (lua_isnil(m_lua, -1))
        return (result);


    /*
     * Are we passing an argument?
     */
    bool has_arg = (argument == "") ? false : true ;

    /*
     * Call the function - either passing in the argument, or not.
     */
    int ret = 0;

    if (has_arg)
    {
        lua_pushstring(m_lua, argument.c_str());
        ret = lua_pcall(m_lua, 1, 1, 0);
    }
    else
        ret = lua_pcall(m_lua, 0, 1, 0);

    /*
     * Handle any error that might have raised.
     */
    if (ret != 0)
    {
        if (lua_isstring(m_lua, -1))
        {
            /*
             * The error message will be on the stack..
             */
            char *err = strdup(lua_tostring(m_lua, -1));
            lua_pop(m_lua, 1);

            on_error(err);

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


/*
 * Return the (string) contents of a variable.
 * Used for our test suite only.
 */
std::string CLua::get_variable(std::string name)
{
    CLuaLog("get_variable(" + name + ")");

    lua_getglobal(m_lua, name.c_str());

    if (lua_isnil(m_lua, -1))
        return "";

    if (lua_isstring(m_lua, -1))
        return lua_tostring(m_lua, -1);

    return ("");
}


/*
 * Call the given sort method, with the specified
 * result.
 */
bool CLua::call_sort(std::string method, std::shared_ptr<CMessage> a, std::shared_ptr<CMessage> b)
{
    CLuaLog("call_sort(" + method + ")");

    method = "compare_by_" + method;

    /*
     * Call the function - if it exists.
     */
    lua_getglobal(m_lua, method.c_str());

    if (lua_isnil(m_lua, -1))
    {
        on_error("The function isn't defined: " + method);
        return false;
    }

    /**
     * OK we've pushed the method.  Now push the args.
     */
    push_cmessage(m_lua, a);
    push_cmessage(m_lua, b);

    if (lua_pcall(m_lua, 2, 1, 0) != 0)
    {
        if (lua_isstring(m_lua, -1))
        {
            /*
             * The error message will be on the stack..
             */
            char *err = strdup(lua_tostring(m_lua, -1));
            lua_pop(m_lua, 1);
            on_error(err);

            return false;
        }
    }

    bool ret = lua_toboolean(m_lua, -1);
    return (ret);
}


/*
 * Call a Lua function with a string argument, and return
 * the single string it will return.
 *
 * If the function returns nothing, or a non-string, return "".
 *
 */
std::string CLua::function2string(std::string function, std::string input)
{
    CLuaLog("function2string(" + function + "," + input + ")");

    /*
     * Get the function - if it doesn't exist we're done.
     */
    lua_getglobal(m_lua, function.c_str());

    if (lua_isnil(m_lua, -1))
        return ("");


    if (input.empty())
        lua_pushstring(m_lua, "");
    else
        lua_pushstring(m_lua, input.c_str());

    /*
     * Call the function - and handle any error.
     */
    if (lua_pcall(m_lua, 1, 1, 0) != 0)
    {
        if (lua_isstring(m_lua, -1))
        {
            /*
             * The error message will be on the stack..
             */
            char *err = strdup(lua_tostring(m_lua, -1));
            lua_pop(m_lua, 1);

            on_error(err);

            /*
             * Avoid a leak.
             */
            free(err);
        }
        else
            lua_pop(m_lua, 1);

        return ("");
    }

    /*
     * Now get the table we expected.
     */
    if (lua_isstring(m_lua, -1))
    {
        const char *result = lua_tostring(m_lua, -1);
        return (result);
    }
    else
    {
        return "";
    }

}


/**
 * Lookup a key-binding.
 */
std::string CLua::keybinding(std::string mode, std::string key)
{
    CLuaLog("keybinding(" + mode + "," + key + ")");

    /*
     * Get the function.
     */
    lua_getglobal(m_lua, "lookup_key");
    lua_pushstring(m_lua, mode.c_str());
    lua_pushstring(m_lua, key.c_str());

    /*
     * Call
     */
    if (lua_pcall(m_lua, 2, 1, 0) != 0)
    {
        if (lua_isstring(m_lua, -1))
        {
            /*
             * The error message will be on the stack..
             */
            char *err = strdup(lua_tostring(m_lua, -1));
            lua_pop(m_lua, 1);
            on_error(err);

            /*
             * Avoid a leak.
             */
            free(err);
        }
        else
            lua_pop(m_lua, 1);
    }

    const char *res = lua_tostring(m_lua, -1);
    std::string out = "";

    if (res != NULL)
        out = res ;

    return (out);
}
