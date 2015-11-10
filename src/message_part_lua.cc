/*
 * message_part_lua.cc - Export our MIME-part object to Lua.
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





/**
 * Push a CMessagePart pointer onto the Lua stack.
 */
void push_cmessagepart(lua_State * l, std::shared_ptr<CMessagePart> part)
{
    /**
     * Allocate a new object.
     */
    void *ud = lua_newuserdata(l, sizeof(std::shared_ptr<CMessagePart>*));

    if (!ud)
    {
        /* Error - couldn't allocate the memory */
        return;
    }

    /* We can't just do *(shared_ptr<...> *)ud = shared_ptr<>... since
     * it will try to call the assignment operator on the object at *ud,
     * but there isn't one (so it tries to free random junk).
     *
     * Instead, construct the new shared pointer in the memory we've just
     * allocated.
     */
    std::shared_ptr<CMessagePart> *udata = new(ud) std::shared_ptr<CMessagePart>();

    /*
     * Now that we have a valid shared_ptr pointing to nothing, we can
     * assign the final value to it.
     */
    *udata = part;

    luaL_getmetatable(l, "luaL_CMessagePart");
    lua_setmetatable(l, -2);
}


std::shared_ptr<CMessagePart> l_CheckCMessagePart(lua_State * l, int n)
{
    void *ud = luaL_checkudata(l, n, "luaL_CMessagePart");

    if (ud)
    {
        /* Get a pointer to the shared_ptr object */
        std::shared_ptr<CMessagePart> *ud_msg = static_cast<std::shared_ptr<CMessagePart> *>(ud);

        /* Return a copy (of the pointer) */
        return *ud_msg;
    }
    else
    {
        /* otherwise a null pointer */
        return std::shared_ptr<CMessagePart>();
    }
}

int l_CMessagePart_content(lua_State * l)
{
    std::shared_ptr<CMessagePart> foo = l_CheckCMessagePart(l, 1);

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
    std::shared_ptr<CMessagePart> foo = l_CheckCMessagePart(l, 1);
    lua_pushstring(l, foo->filename().c_str());
    return 1;
}

int l_CMessagePart_is_attachment(lua_State * l)
{
    std::shared_ptr<CMessagePart> foo = l_CheckCMessagePart(l, 1);

    if (foo->is_attachment())
        lua_pushboolean(l, 1);
    else
        lua_pushboolean(l, 0);

    return 1;
}

int l_CMessagePart_size(lua_State * l)
{
    std::shared_ptr<CMessagePart> foo = l_CheckCMessagePart(l, 1);

    lua_pushinteger(l, foo->content_size());
    return 1;
}

int l_CMessagePart_type(lua_State * l)
{
    std::shared_ptr<CMessagePart> foo = l_CheckCMessagePart(l, 1);
    lua_pushstring(l, foo->type().c_str());
    return 1;
}

int l_CMessagePart_destructor(lua_State * l)
{
    void *ud = luaL_checkudata(l, 1, "luaL_CMessagePart");

    if (ud)
    {
        /* Get a pointer to the shared_ptr object */
        std::shared_ptr<CMessagePart> *ud_msg = static_cast<std::shared_ptr<CMessagePart> *>(ud);

        /* We need to destruct the pointer in place; it will decrement
         * the reference count as usual.  After this the user data object
         * becomes just plain memory again. */
        ud_msg->~shared_ptr<CMessagePart>();
    }

    return 0;
}

void InitMessagePart(lua_State * l)
{
    luaL_Reg sFooRegs[] =
    {
        {"content", l_CMessagePart_content},
        {"filename", l_CMessagePart_filename},
        {"is_attachment", l_CMessagePart_is_attachment},
        {"size", l_CMessagePart_size},
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
