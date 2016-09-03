/*
 * lua.h - Wrapper for the lua interpreter.
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


#pragma once


extern "C"
{
#include <lua.h>
#include <lauxlib.h>
#include <lualib.h>
}

#include <vector>
#include <string>

#include "logger.h"
#include "message_lua.h"
#include "observer.h"
#include "singleton.h"

/**
 * A singleton class holding our Lua interpreter state.
 *
 * This class also implements the observer-pattern, responding to
 * changes in the `CConfig` class.
 *
 */
class CLua : public Singleton<CLua>, public Observer
{
    /**
     * Our debug-helper needs access to our Lua handle to
     * determine the size of the Lua stack.
     */
    friend class CLuaLog;

public:
    CLua();
    ~CLua();

public:

    /**
     * Populate "args" - the command-line arguments which are exposed to Lua.
     */
    void set_args(char *argv[], int argc);

    /**
     * Load the specified Lua file, and evaluate it.
     *
     * If there is an error loading the file then the program will abort,
     * and the error message will be shown to the user.
     */
    void load_file(std::string filename);

    /**
     * Evaluate the given string.
     *
     * Return true on success.  False on error.
     */
    bool execute(std::string lua);

    /**
     * Does the specified function exist (in lua)?
     */
    bool function_exists(std::string function);

    /**
     * Get the known-bindings of the keymap in the given mode.
     *
     * This just returns the names of the keys, rather than their values.
     */
    std::vector<std::string> bindings(std::string mode);

    /**
     * This method is called when a configuration key changes,
     * via our observer implementation.
     */
    void update(std::string key_name);

    /**
     * Call a Lua function which will return a table of text.
     *
     * If the argument is not equal to "" then it will be given as the
     * argument to the specified Lua function.
     */
    std::vector<std::string> function2table(std::string function);

    /**
     * Call a Lua function which will return a table of text.
     *
     * If the argument is not equal to "" then it will be given as the
     * argument to the specified Lua function.
     */
    std::vector<std::string> functiona2table(std::string function, std::string arugment);

    /**
     * Call the user "on_error" function with given error message.
     */
    void on_error(std::string msg);

    /**
     * Return the (string) contents of a variable.
     * Used for our test suite only.
     */
    std::string get_variable(std::string name);

    /**
     * Call a Lua function with a string argument, and return
     * the single string it will return.
     *
     * If the function returns nothing, or a non-string, return "".
     *
     * TODO: Rethink this.
     */
    std::string function2string(std::string function, std::string input);


    /**
     * Lookup a key-binding.
     */
    std::string keybinding(std::string mode, std::string key);

    /**
     * Append to package.path
     */
    void append_to_package_path(std::string);

private:

    /**
     * The handle to the Lua interpreter.
     */
    lua_State * m_lua;

};


/*
 * This is a temporary(?) logging class designed to help me track
 * down leaks in the Lua stack.
 *
 * The lua leaks are what stops displays from operating correctly
 * when luajit is being used.
 *
 */
class CLuaLog
{
public:
    CLuaLog(std::string name)
    {
        m_name = name;

        /*
         * We're going to output padding to show nesting level.
         */
        std::string tmp = "";

        for (int i = 0 ; i < m_nest ; i++)
            tmp += " ";

        tmp += "enter:" + m_name;
        tmp += " ";
        tmp += "stack-depth:" + std::to_string(depth());

        CLogger *x = CLogger::instance();
        x->log("lua", "%s", tmp.c_str());

        /*
         * Bump nesting level.
         */
        m_nest += 1;
    };

    ~CLuaLog()
    {
        m_nest -= 1;

        /*
         * We're going to output padding to show nesting level.
         */
        std::string tmp = "";

        for (int i = 0 ; i < m_nest ; i++)
            tmp += " ";

        tmp += "exit:" + m_name;
        tmp += " ";
        tmp += "stack-depth:" + std::to_string(depth());


        CLogger *x = CLogger::instance();
        x->log("lua", "%s", tmp.c_str());

        /*
         * Ensure we don't go negative.
         */
        if (m_nest < 0)
            m_nest = 0;
    };

    int depth()
    {
        CLua *lua = CLua::instance();
        return (lua_gettop(lua->m_lua));
    };

public:
    static int m_nest;
    std::string m_name;
};
