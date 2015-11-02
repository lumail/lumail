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


extern "C"
{
#include <lua.h>
#include <lauxlib.h>
#include <lualib.h>
}
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
#include "maildir.h"
#include "message.h"
#include "message_lua.h"




/**
 * Push a CMaildr pointer onto the Lua stack.
 */
void push_cmaildir(lua_State * l, std::shared_ptr<CMaildir> maildir)
{
    /**
     * Allocate a new object.
     */
    void *ud = lua_newuserdata(l, sizeof(std::shared_ptr<CMaildir>*));

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
 * Binding for CMaildir
 */
int l_CMaildir_constructor(lua_State * l)
{
    const char *path = luaL_checkstring(l, 1);

    push_cmaildir(l, std::shared_ptr<CMaildir>(new CMaildir(path)));

    return 1;
}



/**
 * Test that the object is a std::shared_ptr<CMaildir>.
 */
std::shared_ptr<CMaildir> l_CheckCMaildir(lua_State * l, int n)
{
    void *ud = luaL_checkudata(l, 1, "luaL_CMaildir");

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
 * Return the path of this maildir object.
 */
int l_CMaildir_path(lua_State * l)
{
    std::shared_ptr<CMaildir> foo = l_CheckCMaildir(l, 1);
    lua_pushstring(l, foo->path().c_str());
    return 1;
}



/**
 * Count the messages in the directory.
 */
int l_CMaildir_total_messages(lua_State * l)
{
    std::shared_ptr<CMaildir> foo = l_CheckCMaildir(l, 1);
    lua_pushinteger(l, foo->total_messages());
    return 1;
}



/**
 * Count the unread messages in the maildir-directory.
 */
int l_CMaildir_unread_messages(lua_State * l)
{
    std::shared_ptr<CMaildir> foo = l_CheckCMaildir(l, 1);
    lua_pushinteger(l, foo->unread_messages());
    return 1;
}



/**
 * Does the maildir exist?
 */
int l_CMaildir_exists(lua_State * l)
{
    std::shared_ptr<CMaildir> foo = l_CheckCMaildir(l, 1);

    int n = lua_gettop(l);

    if (n != 1)
    {
        fprintf(stderr, "GOT A STRING?!\n");
    }

    if (foo->is_maildir())
        lua_pushboolean(l, 1);
    else
        lua_pushboolean(l, 0);

    return 1;
}



/**
 * Return a C++ CMessage object for each message.
 *
 * Suspect this is broken - but it seems to work.
 */
int l_CMaildir_messages(lua_State * l)
{
    std::shared_ptr<CMaildir> foo = l_CheckCMaildir(l, 1);

    std::vector < std::string > tmp = foo->messages();

    lua_createtable(l, tmp.size(), 0);
    int i = 0;

    for (std::vector < std::string >::iterator it = tmp.begin();
            it != tmp.end(); ++it)
    {
        push_cmessage(l, std::shared_ptr<CMessage>(new CMessage(*it)));
        lua_rawseti(l, -2, i + 1);

        i++;
    }

    return 1;
}



/**
 * Destructor.
 */
int l_CMaildir_destructor(lua_State * l)
{
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
 * Get the current maildirs, paying attention to the `maildir.limit`
 * setting.
 */
int current_maildirs(lua_State * L)
{
    CGlobalState *global = CGlobalState::instance();
    std::vector<std::shared_ptr<CMaildir>> maildirs = global->get_maildirs();

    lua_createtable(L, maildirs.size(), 0);
    int i = 0;

    for (std::vector<std::shared_ptr<CMaildir>>::iterator it = maildirs.begin();
            it != maildirs.end(); ++it)
    {
        std::shared_ptr<CMaildir> cur = (*it);
        push_cmaildir(L, cur);

        lua_rawseti(L, -2, i + 1);
        i++;
    }

    return 1;

}

/**
 * Somebody set us up the mapping.
 */
void InitMaildir(lua_State * l)
{
    luaL_Reg sFooRegs[] =
    {
        {"new", l_CMaildir_constructor},
        {"path", l_CMaildir_path},
        {"total_messages", l_CMaildir_total_messages},
        {"unread_messages", l_CMaildir_unread_messages},
        {"messages", l_CMaildir_messages},
        {"exists", l_CMaildir_exists},
        {"__gc", l_CMaildir_destructor},
        {NULL, NULL}
    };
    luaL_newmetatable(l, "luaL_CMaildir");

#if LUA_VERSION_NUM == 501
    luaL_register(l, NULL, sFooRegs);
#elif LUA_VERSION_NUM == 502
    luaL_setfuncs(l, sFooRegs, 0);
#else
#error unsupported Lua version
#endif

    lua_pushvalue(l, -1);
    lua_setfield(l, -1, "__index");
    lua_setglobal(l, "Maildir");


    /**
     * Now add in the static method.
     */
    lua_pushcfunction(l, current_maildirs);
    lua_setglobal(l, "current_maildirs");
}
