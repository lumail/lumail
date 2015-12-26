/*
 * mime_lua.cc - MIME functions bound to lua.
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

#include "mime.h"

/**
 * @file mime_lua.cc
 *
 * This file implements the trivial exporting of a MIME-class to Lua.
 *
 * There is only a single method implemented, and usage looks like this:
 *
 *<code>
 *   -- Get the MIME-type of /etc/passwd.<br />
 *   local m = MIME:type( "/etc/passwd" )<br />
 * <br/>
 *   -- Show it<br />
 *   Panel:append( "MIME type is " .. m )<br />
 *</code>
 *
 */



/**
 * Get the MIME-type of the given file.
 */
int l_CMime_type(lua_State * L)
{
    CMime *mime = CMime::instance();

    const char *file = lua_tostring(L, 1);

    std::string ctype = mime->type(file);

    if (ctype.empty())
        lua_pushnil(L);
    else
        lua_pushstring(L, ctype.c_str());

    return 1;
}


/**
 * Register the global `MIME` object to the Lua environment,
 * and setup our public methods upon which the user may operate.
 */
void InitMIME(lua_State * l)
{
    luaL_Reg sFooRegs[] =
    {
        {"type",  l_CMime_type},
        {NULL,      NULL}
    };
    luaL_newmetatable(l, "luaL_CMime");

#if LUA_VERSION_NUM == 501
    luaL_register(l, NULL, sFooRegs);
#elif LUA_VERSION_NUM == 502 || LUA_VERSION_NUM == 503
    luaL_setfuncs(l, sFooRegs, 0);
#else
#error We are only tested under Lua 5.1, 5.2, or 5.3.
#endif

    lua_pushvalue(l, -1);
    lua_setfield(l, -1, "__index");
    lua_setglobal(l, "MIME");
}
