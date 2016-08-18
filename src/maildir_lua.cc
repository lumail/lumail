/*
 * maildir_lua.cc - Export our Maildir object to Lua.
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


#include <algorithm>
#include <cstdlib>
#include <dirent.h>
#include <fcntl.h>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string.h>
#include <string>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <unordered_map>
#include <vector>
#include <gmime/gmime.h>


#include "file.h"
#include "global_state.h"
#include "lua.h"
#include "maildir.h"
#include "message.h"
#include "message_lua.h"


/**
 * @file maildir_lua.cc
 *
 * This file implements the trivial exporting of our CMaildir class,
 * implemented in C++, to Lua.  Lua-usage looks something like this:
 *
 *<code>
 *   -- Create a maildir, and count messages <br/>
 *   local m = Maildir.new( "/home/steve/Maildir/.foo.com" ) <br/>
 *   local u = m:unread_messages() <br/>
 *   local t = m:total_messages() <br/>
 *</code>
 *
 */


/**
 * Push a CMaildir pointer onto the Lua stack.
 */
void push_cmaildir(lua_State * l, std::shared_ptr<CMaildir> maildir)
{
    CLuaLog("push_cmaildir");

    /*
     * Allocate a new object.
     */
    void *ud = lua_newuserdata(l, sizeof(std::shared_ptr<CMaildir>));

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
    std::shared_ptr<CMaildir> *udata = new(ud) std::shared_ptr<CMaildir>();

    /*
     * Now that we have a valid shared_ptr pointing to nothing, we can
     * assign the final value to it.
     */
    *udata = maildir;

    luaL_getmetatable(l, "luaL_CMaildir");
    lua_setmetatable(l, -2);
}


/**
 * Implementation for CMaildir.new
 */
int l_CMaildir_constructor(lua_State * l)
{
    CLuaLog("l_CMaildir_constructor");

    const char *path = luaL_checkstring(l, 1);

    push_cmaildir(l, std::shared_ptr<CMaildir>(new CMaildir(path)));

    return 1;
}


/**
 * Test that the object on the Lua stack is a std::shared_ptr<CMaildir>.
 */
std::shared_ptr<CMaildir> l_CheckCMaildir(lua_State * l, int n)
{
    void *ud = luaL_checkudata(l, n, "luaL_CMaildir");

    if (ud)
    {
        /* Get a pointer to the shared_ptr object */
        std::shared_ptr<CMaildir> *ud_msg = static_cast<std::shared_ptr<CMaildir> *>(ud);

        /* Return a copy (of the pointer) */
        return *ud_msg;
    }
    else
    {
        /* otherwise a null pointer */
        return std::shared_ptr<CMaildir>();
    }
}


/**
 * Implementation of Maildir:is_imap()
 */
int l_CMaildir_is_imap(lua_State * l)
{
    CLuaLog("l_CMaildir_is_imap");

    std::shared_ptr<CMaildir> foo = l_CheckCMaildir(l, 1);

    if (foo->is_imap())
        lua_pushboolean(l , 1);
    else
        lua_pushboolean(l , 1);

    return 1;
}



/**
 * Implementation of Maildir:is_maildir()
 */
int l_CMaildir_is_maildir(lua_State * l)
{
    CLuaLog("l_CMaildir_is_maildir");

    std::shared_ptr<CMaildir> foo = l_CheckCMaildir(l, 1);

    if (foo->is_maildir())
        lua_pushboolean(l , 1);
    else
        lua_pushboolean(l , 1);

    return 1;
}

/**
 * Implementation of Maildir:path()
 */
int l_CMaildir_path(lua_State * l)
{
    CLuaLog("l_CMaildir_path");

    std::shared_ptr<CMaildir> foo = l_CheckCMaildir(l, 1);
    lua_pushstring(l, foo->path().c_str());
    return 1;
}



/**
 * Implementation of Maildir:save_message()
 */
int l_CMaildir_save_message(lua_State * l)
{
    CLuaLog("l_CMaildir_save_message");

    std::shared_ptr<CMaildir> maildir = l_CheckCMaildir(l, 1);
    std::shared_ptr<CMessage> message = l_CheckCMessage(l, 2);

    bool ret = maildir->saveMessage(message);

    if (ret)
        lua_pushboolean(l , 1);
    else
        lua_pushboolean(l , 0);

    return 1;
}

/**
 * Implementation of Maildir:total_messages()
 */
int l_CMaildir_total_messages(lua_State * l)
{
    CLuaLog("l_CMaildir_total_messages");

    std::shared_ptr<CMaildir> foo = l_CheckCMaildir(l, 1);
    lua_pushinteger(l, foo->total_messages());
    return 1;
}



/**
 * Implementation of Maildir:unread_messages()
 */
int l_CMaildir_unread_messages(lua_State * l)
{
    CLuaLog("l_CMaildir_unread_messages");

    std::shared_ptr<CMaildir> foo = l_CheckCMaildir(l, 1);
    lua_pushinteger(l, foo->unread_messages());
    return 1;
}


/**
 * Implementation of Maildir:messages()
 *
 * Return a Lua CMessage object for each message.
 */
int l_CMaildir_messages(lua_State * l)
{
    CLuaLog("l_CMaildir_messages");

    std::shared_ptr<CMaildir> foo = l_CheckCMaildir(l, 1);

    CMessageList tmp = foo->getMessages();

    lua_createtable(l, tmp.size(), 0);
    int i = 0;

    for (CMessageList::iterator it = tmp.begin(); it != tmp.end(); ++it)
    {
        push_cmessage(l, (*it));
        lua_rawseti(l, -2, i + 1);

        i++;
    }

    return 1;
}


/**
 * Implementation of Maildir:mtime()
 */
int l_CMaildir_mtime(lua_State *l)
{
    CLuaLog("l_CMaildir_mtime");

    /*
     * Get the path.
     */
    std::shared_ptr<CMaildir> foo = l_CheckCMaildir(l, 1);
    int mtime = foo->last_modified();
    lua_pushinteger(l, mtime);
    return 1;
}


/**
 * Destructor.
 */
int l_CMaildir_destructor(lua_State * l)
{
    CLuaLog("l_CMaildir_destructor");

    void *ud = luaL_checkudata(l, 1, "luaL_CMaildir");

    if (ud)
    {
        /* Get a pointer to the shared_ptr object */
        std::shared_ptr<CMaildir> *ud_msg = static_cast<std::shared_ptr<CMaildir> *>(ud);

        /* We need to destruct the pointer in place; it will decrement
         * the reference count as usual.  After this the user data object
         * becomes just plain memory again. */
        ud_msg->~shared_ptr<CMaildir>();
    }

    return 0;
}


/**
 * Equality-test - this is bound to the Lua meta-method `__eq` such that two
 * maildirs may be tested for equality.
 *
 * Two maildr-objects are considered identical if they refer to the
 * same folder on-disk.
 */
int l_CMaildir_equality(lua_State * l)
{
    CLuaLog("l_CMaildir_equality");

    std::shared_ptr<CMaildir> one = l_CheckCMaildir(l, 1);
    std::shared_ptr<CMaildir> two = l_CheckCMaildir(l, 2);

    if (one->path() == two->path())
        lua_pushboolean(l , 1);
    else
        lua_pushboolean(l , 0);

    return 1;
}


/**
 * Register the global `Maildir` object to the Lua environment, and
 * setup our public methods upon which the user may operate.
 */
void InitMaildir(lua_State * l)
{
    luaL_Reg sFooRegs[] =
    {
        {"__gc", l_CMaildir_destructor},
        {"__eq", l_CMaildir_equality},
        {"is_imap", l_CMaildir_is_imap},
        {"is_maildir", l_CMaildir_is_maildir},
        {"messages", l_CMaildir_messages},
        {"mtime", l_CMaildir_mtime},
        {"new", l_CMaildir_constructor},
        {"path", l_CMaildir_path},
        {"save_message", l_CMaildir_save_message},
        {"total_messages", l_CMaildir_total_messages},
        {"unread_messages", l_CMaildir_unread_messages},
        {NULL, NULL}
    };
    luaL_newmetatable(l, "luaL_CMaildir");

#if LUA_VERSION_NUM == 501
    luaL_register(l, NULL, sFooRegs);
#elif LUA_VERSION_NUM == 502 || LUA_VERSION_NUM == 503
    luaL_setfuncs(l, sFooRegs, 0);
#else
#error We are only tested under Lua 5.1, 5.2, or 5.3.
#endif

    lua_pushvalue(l, -1);
    lua_setfield(l, -1, "__index");
    lua_setglobal(l, "Maildir");

}
