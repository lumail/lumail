/*
 * utf_lua.cc - UTF functions bound to lua.
 *
 * This file is part of lumail - http://lumail.org/
 *
 * Copyright (c) 2017 by Steve Kemp.  All rights reserved.
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


#include <unistd.h>

#include "lua.h"
#include "util.h"

/**
 * @file utf_lua.cc
 *
 * This file implements the trivial exporting of a UTF-class to Lua.
 *
 * There is only a single method implemented, and usage looks like this:
 *
 *<code>
 *   -- Count the length of the given string<br />
 *   local h = UTF:len( "string" )<br />
 *</code>
 *
 */



/**
 * Get the length of the given string.
 */
int l_CUtf_len(lua_State * L)
{
    CLuaLog("l_CUtf_len");

    /*
     * the input
     */
    const char *str = luaL_checkstring(L, 2);

    std::string tmp(str);

    int max = (int)tmp.length();

    int len = 0;

    for (int i = 0; i < max; i++)
    {
        const char byte = tmp.at(i);
        int size = dsutil_utf8_charlen(byte);

        if (size >= 1)
            len ++;
    }

    lua_pushinteger(L, len);
    return 1;
}


/**
 * Export the UTF object to Lua, this only contains the single static
 * method `len`.
 */
void InitUtf(lua_State * l)
{
    luaL_Reg sFooRegs[] =
    {
        {"len",  l_CUtf_len},
        {NULL,      NULL}
    };
    luaL_newmetatable(l, "luaL_CUtf");

#if LUA_VERSION_NUM == 501
    luaL_register(l, NULL, sFooRegs);
#elif LUA_VERSION_NUM == 502 || LUA_VERSION_NUM == 503
    luaL_setfuncs(l, sFooRegs, 0);
#else
#error We are only tested under Lua 5.1, 5.2, or 5.3.
#endif

    lua_pushvalue(l, -1);
    lua_setfield(l, -1, "__index");
    lua_setglobal(l, "UTF");
}
