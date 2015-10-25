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


extern "C"
{
#include <lua.h>
#include <lauxlib.h>
#include <lualib.h>
}

#include "message_part.h"


CMessagePart * l_CheckCMessagePart(lua_State * l, int n)
{
    return *(CMessagePart **) luaL_checkudata(l, n, "luaL_CMessagePart");
}

int l_CMessagePart_content(lua_State * l)
{
    CMessagePart *foo = l_CheckCMessagePart(l, 1);

    void *content = foo->content();
    size_t content_size = foo->content_size();

    if (content_size > 0)
        lua_pushlstring(l, (char *) content, content_size);
    else
        lua_pushnil(l);

    return 1;
}

int l_CMessagePart_filename(lua_State * l)
{
    CMessagePart *foo = l_CheckCMessagePart(l, 1);

    lua_pushstring(l, foo->filename().c_str());
    return 1;
}

int l_CMessagePart_is_attachment(lua_State * l)
{
    CMessagePart *foo = l_CheckCMessagePart(l, 1);

    if (foo->is_attachment())
        lua_pushboolean(l, 1);
    else
        lua_pushboolean(l, 0);

    return 1;
}

int l_CMessagePart_type(lua_State * l)
{
    CMessagePart *foo = l_CheckCMessagePart(l, 1);

    lua_pushstring(l, foo->type().c_str());
    return 1;
}

int l_CMessagePart_destructor(lua_State * l)
{
    CMessagePart *foo = l_CheckCMessagePart(l, 1);
    delete foo;
    return 0;
}

void InitMessagePart(lua_State * l)
{
    luaL_Reg sFooRegs[] =
    {
        {"content", l_CMessagePart_content},
        {"filename", l_CMessagePart_filename},
        {"is_attachment", l_CMessagePart_is_attachment},
        {"type", l_CMessagePart_type},
        {"__gc", l_CMessagePart_destructor},
        {NULL, NULL}
    };
    luaL_newmetatable(l, "luaL_CMessagePart");

#if LUA_VERSION_NUM == 501
    luaL_register(l, NULL, sFooRegs);
#elif LUA_VERSION_NUM == 502
    luaL_setfuncs(l, sFooRegs, 0);
#else
#error unsupported Lua version
#endif

    lua_pushvalue(l, -1);
    lua_setfield(l, -1, "__index");
    lua_setglobal(l, "MessagePart");
}
