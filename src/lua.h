/*
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


/**
 * Only include this header one time.
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


/**
 * A singleton class holding our Lua interpreter state.
 */
class CLua
{
private:
    CLua();
    ~CLua();

public:

    /**
     * Get access to this singleton instance.
     */
    static CLua *instance();

    /**
     * Load the specified Lua file, and evaluate it.
     *
     * Return true on success.  False on error.
     */
    bool load_file(std::string filename);

    /**
     * Evaluate the given string.
     *
     * Return true on success.  False on error.
     */
    bool execute(std::string lua);

    /**
     * Lookup a key in a nested table structure - used for keyboard lookups.
     */
    char *get_nested_table(std::string table, const char *key, const char *subkey);

    /**
     * Call `on_complete` to complete a string.
     */
    std::vector<std::string> get_completions(std::string token);

    /**
     * HACK - TODO - Fix
     */
    lua_State *state()
    {
        return m_lua;
    }

private:

    /**
     * The handle to the Lua interpreter.
     */
    lua_State * m_lua;

};
