/*
 * cache_lua.cc - Export our Cache-object to Lua.
 *
 * This file is part of lumail - http://lumail.org/
 *
 * Copyright (c) 2016 by Steve Kemp.  All rights reserved.
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


#include "lua.h"
#include "cache.h"


/**
 * @file cache_lua.cc
 *
 * This file implements the trivial exporting of our CCache class,
 * implemented in C++, to Lua.  Lua-usage looks something like this:
 *
 *<code>
 *   -- Create a message object and get the MIME-parts <br/>
 *   local c = Cache.new( "/path/to/cache" ) <br/>
 *   c:set( "foo", "bar") <br/>
 *   print( c:get( "foo" ) ) <br/>
 *</code>
 *
 */



/**
 * Push a CCache pointer onto the Lua stack.
 */
void push_ccache(lua_State * l, std::shared_ptr<CCache> part)
{
    CLuaLog("push_ccache");

    /*
     * Allocate a new object.
     */
    void *ud = lua_newuserdata(l, sizeof(std::shared_ptr<CCache>));

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
    std::shared_ptr<CCache> *udata = new(ud) std::shared_ptr<CCache>();

    /*
     * Now that we have a valid shared_ptr pointing to nothing, we can
     * assign the final value to it.
     */
    *udata = part;

    luaL_getmetatable(l, "luaL_CCache");
    lua_setmetatable(l, -2);
}


/**
 * Test that the object is a std::shared_ptr<CCache>.
 */
std::shared_ptr<CCache> l_CheckCCache(lua_State * l, int n)
{
    CLuaLog("l_CheckCCache");

    void *ud = luaL_checkudata(l, n, "luaL_CCache");

    if (ud)
    {
        /* Get a pointer to the shared_ptr object */
        std::shared_ptr<CCache> *ud_msg = static_cast<std::shared_ptr<CCache> *>(ud);

        /* Return a copy (of the pointer) */
        return *ud_msg;
    }
    else
    {
        /* otherwise a null pointer */
        return std::shared_ptr<CCache>();
    }
}


/**
 * Implementation for Cache.new
 */
int l_CCache_constructor(lua_State * l)
{
    CLuaLog("l_CCache_constructor");
    push_ccache(l, std::shared_ptr<CCache>(new CCache()));
    return 1;
}

/**
 * Implementation of Cache:get()
 */
int l_CCache_get(lua_State * l)
{
    CLuaLog("l_CCache_get");

    std::shared_ptr<CCache> foo = l_CheckCCache(l, 1);

    const char *key = luaL_checkstring(l, 2);
    std::string value = foo->get(key);

    if (value.empty())
    {
        lua_pushnil(l);
    }
    else
    {
        lua_pushstring(l, value.c_str());

    }

    return 1;
}


/**
 * Implementation of Cache:set()
 */
int l_CCache_set(lua_State * l)
{
    CLuaLog("l_CCache_set");

    const char *key = luaL_checkstring(l, 2);
    const char *val = luaL_checkstring(l, 3);

    std::shared_ptr<CCache> foo = l_CheckCCache(l, 1);
    foo->set(key, val);
    return 0;
}


/**
 * Destructor
 */
int l_CCache_destructor(lua_State * l)
{
    CLuaLog("l_CCache_destructor");

    void *ud = luaL_checkudata(l, 1, "luaL_CCache");

    if (ud)
    {
        /* Get a pointer to the shared_ptr object */
        std::shared_ptr<CCache> *ud_msg = static_cast<std::shared_ptr<CCache> *>(ud);

        /* We need to destruct the pointer in place; it will decrement
         * the reference count as usual.  After this the user data object
         * becomes just plain memory again. */
        ud_msg->~shared_ptr<CCache>();
    }

    return 0;
}



/**
 * Register the global `MessagePart` object to the Lua environment, and
 * setup our public methods upon which the user may operate.
 */
void InitCache(lua_State * l)
{
    luaL_Reg sFooRegs[] =
    {
        {"get", l_CCache_get},
        {"new", l_CCache_constructor},
        {"set", l_CCache_set},
        {"__gc", l_CCache_destructor},
        {NULL, NULL}
    };
    luaL_newmetatable(l, "luaL_CCache");

#if LUA_VERSION_NUM == 501
    luaL_register(l, NULL, sFooRegs);
#elif LUA_VERSION_NUM == 502 || LUA_VERSION_NUM == 503
    luaL_setfuncs(l, sFooRegs, 0);
#else
#error We are only tested under Lua 5.1, 5.2, or 5.3.
#endif

    lua_pushvalue(l, -1);
    lua_setfield(l, -1, "__index");
    lua_setglobal(l, "Cache");
}
