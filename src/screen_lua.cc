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



#include "message_lua.h"
#include "global_state.h"
#include "screen.h"




/**
 * Clear the screen.
 */
int l_CScreen_clear(lua_State * l)
{
    CScreen *foo = CScreen::instance();
    foo->clear();
    return 0;
}


/**
 * Exit our main event-loop.
 */
int l_CScreen_exit(lua_State * l)
{
    CScreen *foo = CScreen::instance();
    foo->exit_main_loop();
    return 0;
}

/**
 * Get the current maildir.
 */
int l_CScreen_maildir(lua_State * l)
{
    /**
     * Get all maildirs.
     */
    CGlobalState *state = CGlobalState::instance();
    std::shared_ptr<CMaildir > current = state->current_maildir();

    if (! current)
        return 0;

    CMaildir **udata = (CMaildir **) lua_newuserdata(l, sizeof(CMaildir *));
    *udata = new CMaildir(current->path());
    luaL_getmetatable(l, "luaL_CMaildir");
    lua_setmetatable(l, -2);

    return 1;
}


/**
 * Get the current message
 */
int l_CScreen_message(lua_State * l)
{
    /**
     * Get all maildirs.
     */
    CGlobalState *state = CGlobalState::instance();
    std::shared_ptr<CMessage> m = state->current_message();

    push_cmessage(l, m);

    return 1;
}


int l_CScreen_select_maildir(lua_State * l)
{
    const int offset = luaL_checkinteger(l, 2);

    /**
     * Get all maildirs.
     */
    CGlobalState *state = CGlobalState::instance();
    std::vector<std::shared_ptr<CMaildir> > maildirs = state->get_maildirs();

    /**
     * Get the one at the index.
     */
    std::shared_ptr<CMaildir> m = maildirs.at(offset);

    /**
     * Update the global-state
     */
    state->set_maildir(m);
    return 0;
}

int l_CScreen_select_message(lua_State * l)
{
    const int offset = luaL_checkinteger(l, 2);

    /**
     * Get the messages
     */
    CGlobalState *global   = CGlobalState::instance();
    CMessageList *messages = global->get_messages();

    /**
     * Get the one at the index.
     */
    std::shared_ptr<CMessage> m = messages->at(offset);

    /**
     * Update the global-state
     */
    global->set_message(m);
    return 0;
}


int
l_CScreen_get_line(lua_State * l)
{
    CScreen *foo = CScreen::instance();
    std::string received = foo->get_line();

    if (received.empty())
        lua_pushnil(l);
    else
        lua_pushstring(l, received.c_str());

    return 1;
}

int
l_CScreen_height(lua_State * l)
{
    CScreen *foo = CScreen::instance();
    lua_pushinteger(l, foo->height());
    return 1;
}

int
l_CScreen_sleep(lua_State * l)
{
    const int delay = luaL_checkinteger(l, 2);
    CScreen *foo = CScreen::instance();
    foo->sleep(delay);
    return 0;
}

int
l_CScreen_width(lua_State * l)
{
    CScreen *foo = CScreen::instance();
    lua_pushinteger(l, foo->width());
    return 1;
}

void
InitScreen(lua_State * l)
{
    luaL_Reg sFooRegs[] =
    {
        {"clear", l_CScreen_clear},
        {"exit",  l_CScreen_exit},
        {"get_line", l_CScreen_get_line},
        {"height", l_CScreen_height},
        {"maildir", l_CScreen_maildir},
        {"message", l_CScreen_message},
        {"select_maildir", l_CScreen_select_maildir},
        {"select_message", l_CScreen_select_message},
        {"sleep", l_CScreen_sleep},
        {"width", l_CScreen_width},
        {NULL, NULL}
    };
    luaL_newmetatable(l, "luaL_CScreen");

#if LUA_VERSION_NUM == 501
    luaL_register(l, NULL, sFooRegs);
#elif LUA_VERSION_NUM == 502
    luaL_setfuncs(l, sFooRegs, 0);
#else
#error unsupported Lua version
#endif

    lua_pushvalue(l, -1);
    lua_setfield(l, -1, "__index");
    lua_setglobal(l, "Screen");

}
