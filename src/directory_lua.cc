/*
 * directory_lua.cc - Export the `Directory` object to Lua.
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


extern "C"
{
#include <lua.h>
#include <lauxlib.h>
#include <lualib.h>
}


#include "directory.h"


/**
 * Does the given directory exist?
 */
int l_CDirectory_exists(lua_State * l)
{
    const char *str = lua_tostring(l, 2);

    if (str != NULL)
    {
        if (CDirectory::exists(str))
            lua_pushboolean(l , 1);
        else
            lua_pushboolean(l , 0);

        return 1;
    }

    return 0;
}


/**
 * Get the entries beneath given directory, return them to Lua.
 */
int l_CDirectory_entries(lua_State * l)
{
    const char *str = lua_tostring(l, 2);
    std::vector<std::string> entries = CDirectory::entries(str);


    lua_newtable(l);

    int i = 1;

    for (auto it = entries.begin(); it != entries.end(); ++it)
    {
        std::string value = (*it);

        lua_pushinteger(l, i);
        lua_pushstring(l, value.c_str());

        lua_settable(l, -3);

        i += 1;
    }

    return 1;
}


/**
 * Export the `Directory` class to Lua.
 *
 * Bind the appropriate methods to that object.
 */
void InitDirectory(lua_State * l)
{
    luaL_Reg sFooRegs[] =
    {
        {"exists",  l_CDirectory_exists},
        {"entries", l_CDirectory_entries},
        {NULL,      NULL}
    };
    luaL_newmetatable(l, "luaL_CDirectory");

#if LUA_VERSION_NUM == 501
    luaL_register(l, NULL, sFooRegs);
#elif LUA_VERSION_NUM == 502
    luaL_setfuncs(l, sFooRegs, 0);
#else
#error unsupported Lua version
#endif

    lua_pushvalue(l, -1);
    lua_setfield(l, -1, "__index");
    lua_setglobal(l, "Directory");
}
