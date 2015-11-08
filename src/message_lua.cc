/*
 * message_lua.cc - Bindings for the CMessage C++ object.
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
#include <gmime/gmime.h>
#include <iostream>
#include <memory>
#include <sstream>
#include <string.h>
#include <string>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <unordered_map>
#include <vector>

#include "file.h"
#include "global_state.h"
#include "message.h"
#include "message_part.h"
#include "message_part_lua.h"


int l_CNet_hostname(lua_State * L);


/**
 * Push a CMessage pointer onto the Lua stack.
 */
void push_cmessage(lua_State * l, std::shared_ptr<CMessage> message)
{
    /**
     * Allocate a new object.
     */
    void *ud = lua_newuserdata(l, sizeof(std::shared_ptr<CMessage>*));

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
    std::shared_ptr<CMessage> *udata = new(ud) std::shared_ptr<CMessage>();

    /*
     * Now that we have a valid shared_ptr pointing to nothing, we can
     * assign the final value to it.
     */
    *udata = message;

    luaL_getmetatable(l, "luaL_CMessage");
    lua_setmetatable(l, -2);
}

/**
 * Binding for CMessage
 */
int l_CMessage_constructor(lua_State * l)
{
    const char *name = luaL_checkstring(l, 1);

    push_cmessage(l, std::shared_ptr<CMessage>(new CMessage(name)));

    return 1;
}

/**
 * Test that the object is a std::shared_ptr<CMessage>.
 */
std::shared_ptr<CMessage> l_CheckCMessage(lua_State * l, int n)
{
    void *ud = luaL_checkudata(l, 1, "luaL_CMessage");

    if (ud)
    {
        /* Get a pointer to the shared_ptr object */
        std::shared_ptr<CMessage> *ud_msg = static_cast<std::shared_ptr<CMessage> *>(ud);

        /* Return a copy (of the pointer) */
        return *ud_msg;
    }
    else
    {
        /* otherwise a null pointer */
        return std::shared_ptr<CMessage>();
    }
}


/**
 * Get the path to the message, on-disk.
 */
int l_CMessage_path(lua_State * l)
{
    std::shared_ptr<CMessage> foo = l_CheckCMessage(l, 1);
    lua_pushstring(l, foo->path().c_str());
    return 1;
}

/**
 * Generate and return a suitable Message-ID.
 */
int l_CMessage_generate_message_id(lua_State *L)
{
    char *name = (char *)"example.org";

    /*
     * Defined in net_lua.cc
     */
    if (l_CNet_hostname(L) == 1)
    {
        name = (char *)lua_tostring(L, -1);
    }

    /*
     * Generate a new ID.
     */
    char *message_id = g_mime_utils_generate_message_id(name);
    std::string result(message_id);
    result = "<" + result + ">";
    g_free(message_id);

    lua_pushstring(L, result.c_str());
    return 1;

}


/**
 * Get the value of a specific header.
 */
int l_CMessage_header(lua_State * l)
{
    std::shared_ptr<CMessage> foo = l_CheckCMessage(l, 1);

    /* Get the header. */
    const char *str = luaL_checkstring(l, 2);
    std::string result = foo->header(str);

    /* set the retulr */
    lua_pushstring(l, result.c_str());
    return 1;

}

/**
 * Return all header names & values.
 */
int l_CMessage_headers(lua_State * l)
{
    /**
     * Get the headers.
     */
    std::shared_ptr<CMessage> foo = l_CheckCMessage(l, 1);
    std::unordered_map < std::string, std::string > headers = foo->headers();


    /**
     * Create the table.
     */
    lua_newtable(l);

    for (auto it = headers.begin(); it != headers.end(); ++it)
    {
        std::string name = it->first;
        std::string value = it->second;

        lua_pushstring(l, name.c_str());

        if (!value.empty())
            lua_pushstring(l, value.c_str());
        else
            lua_pushstring(l, "[EMPTY]");

        lua_settable(l, -3);
    }

    return (1);
}


/**
 * Mark the message as read.
 */
int l_CMessage_mark_read(lua_State *l)
{
    std::shared_ptr<CMessage> foo = l_CheckCMessage(l, 1);
    foo->mark_read();
    return 0;
}

/**
 * Mark the message as unread.
 */
int l_CMessage_mark_unread(lua_State *l)
{
    std::shared_ptr<CMessage> foo = l_CheckCMessage(l, 1);
    foo->mark_unread();
    return 0;
}

/**
 * Return an array of CMessagePart objects to Lua.  These can be inspected
 * as the user wishes.
 *
 * NOTE: CMessagePart is *NOT* creatable via Lua.  This is a good thing.
 *
 */
int l_CMessage_parts(lua_State * l)
{
    std::shared_ptr<CMessage> foo = l_CheckCMessage(l, 1);

    /**
     * Get the parts, and count.
     */
    std::vector<std::shared_ptr<CMessagePart>> parts = foo->get_parts();

    lua_createtable(l, parts.size(), 0);
    int i = 0;

    for (std::vector<std::shared_ptr<CMessagePart>>::iterator it = parts.begin();
            it != parts.end(); ++it)
    {
        std::shared_ptr<CMessagePart> cur = (*it);
        push_cmessagepart(l, cur);
        lua_rawseti(l, -2, i + 1);
        i++;
    }

    return 1;
}


/**
 * Copy the message to a new locatin.
 */
int l_CMessage_copy(lua_State * l)
{
    std::shared_ptr<CMessage> foo = l_CheckCMessage(l, 1);
    std::string dst = luaL_checkstring(l, 2);

    std::vector < std::string > dirs;
    dirs.push_back(dst);
    dirs.push_back(dst + "/cur");
    dirs.push_back(dst + "/tmp");
    dirs.push_back(dst + "/new");

    for (std::vector < std::string >::iterator it = dirs.begin();
            it != dirs.end(); ++it)
    {
        if (!CFile::is_directory(*it))
        {
            lua_pushboolean(l , 0);
            return 1;
        }
    }

    if (foo->copy(dst))
        lua_pushboolean(l , 1);
    else
        lua_pushboolean(l , 0);

    return 1;
}


/**
 * Get/Set the flags.
 */
int l_CMessage_flags(lua_State * l)
{
    std::shared_ptr<CMessage> foo = l_CheckCMessage(l, 1);

    /**
     * Are we setting the flags?
     */
    if (lua_gettop(l) >= 2)
    {
        const char *update = luaL_checkstring(l, 2);
        foo->set_flags(update);
    }

    /**
     * Now get the flags
     */
    lua_pushstring(l, foo->get_flags().c_str());
    return 1;
}

/**
 * Destructor.
 */
int l_CMessage_destructor(lua_State * l)
{
    void *ud = luaL_checkudata(l, 1, "luaL_CMessage");

    if (ud)
    {
        /* Get a pointer to the shared_ptr object */
        std::shared_ptr<CMessage> *ud_msg = static_cast<std::shared_ptr<CMessage> *>(ud);

        /* We need to destruct the pointer in place; it will decrement
         * the reference count as usual.  After this the user data object
         * becomes just plain memory again. */
        ud_msg->~shared_ptr<CMessage>();
    }

    return 0;
}


/**
 * Delete the message.
 */
int l_CMessage_unlink(lua_State * l)
{
    std::shared_ptr<CMessage> foo = l_CheckCMessage(l, 1);

    if (foo)
        foo->unlink();

    /**
     * Update our global-state to reflect the fact that
     * a message has been deleted.
     */
    CGlobalState *global = CGlobalState::instance();
    global->set_message(NULL);
    global->update_messages();

    return 0;
}

void InitMessage(lua_State * l)
{
    luaL_Reg sFooRegs[] =
    {
        {"__gc", l_CMessage_destructor},
        {"copy", l_CMessage_copy},
        {"flags", l_CMessage_flags},
        {"generate_message_id", l_CMessage_generate_message_id},
        {"header", l_CMessage_header},
        {"headers", l_CMessage_headers},
        {"mark_read", l_CMessage_mark_read},
        {"mark_unread", l_CMessage_mark_unread},
        {"new", l_CMessage_constructor},
        {"parts", l_CMessage_parts},
        {"path", l_CMessage_path},
        {"unlink", l_CMessage_unlink},
        {NULL, NULL}
    };
    luaL_newmetatable(l, "luaL_CMessage");

#if LUA_VERSION_NUM == 501
    luaL_register(l, NULL, sFooRegs);
#elif LUA_VERSION_NUM == 502
    luaL_setfuncs(l, sFooRegs, 0);
#else
#error unsupported Lua version
#endif

    lua_pushvalue(l, -1);
    lua_setfield(l, -1, "__index");
    lua_setglobal(l, "Message");
}
