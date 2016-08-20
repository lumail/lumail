/*
 * global_state_lua.cc - Export our global-state to Lua.
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
#include <iostream>

#include "config.h"
#include "global_state.h"
#include "maildir_lua.h"
#include "message_lua.h"
#include "lua.h"
#include "screen.h"


/**
 * @file global_state_lua.cc
 *
 * This file implements the trivial exporting of our CGlobalState class,
 * implemented in C++, to Lua.  Lua-usage looks something like this:
 *
 *<code>
 *   -- Get the currently selected message <br/>
 *   local msg = CGlobal:current_message();
 *</code>
 *
 */


/**
 * Implementation of `Global:maildirs`.
 */
int l_CGlobalState_maildirs(lua_State * L)
{
    CLuaLog("l_CGlobalState_maildirs");

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
 * Implementation of `Global:current_maildir`.
 */
int l_CGlobalState_current_maildir(lua_State * L)
{
    CLuaLog("l_CGlobalState_current_maildir");

    CGlobalState *global = CGlobalState::instance();
    std::shared_ptr<CMaildir> current = global->current_maildir();

    if (current)
        push_cmaildir(L, current);
    else
        lua_pushnil(L);

    return 1;
}



/**
 * Implementation of `Global:select_maildir`.
 */
int l_CGlobalState_select_maildir(lua_State * L)
{
    std::shared_ptr<CMaildir> foo = l_CheckCMaildir(L, 2);
    CGlobalState *global = CGlobalState::instance();

    global->set_maildir(foo);
    return 0;
}


/**
 * Implementation of `Global:current_message`.
 */
int l_CGlobalState_current_message(lua_State * l)
{
    CGlobalState *state = CGlobalState::instance();
    std::shared_ptr<CMessage> m = state->current_message();

    if (m)
        push_cmessage(l, m);
    else
        lua_pushnil(l);

    return 1;
}


/**
 * Implementation of `Global:current_messages`.
 */
int l_CGlobalState_current_messages(lua_State * l)
{
    CGlobalState *global = CGlobalState::instance();
    std::vector<std::shared_ptr<CMessage> > *msgs = global->get_messages();

    lua_createtable(l, msgs->size(), 0);
    int i = 0;

    for (std::vector <std::shared_ptr<CMessage>>::iterator it = msgs->begin();
            it != msgs->end(); ++it)
    {
        std::shared_ptr<CMessage> m = (*it);
        push_cmessage(l, m);
        lua_rawseti(l, -2, i + 1);
        i++;
    }

    return 1;
}



/**
 * Return all the registered view-modes to the caller.
 */
int l_CGlobalState_modes(lua_State * l)
{
    CScreen *screen = CScreen::instance();
    std::vector<std::string> modes = screen->view_modes();

    lua_newtable(l);

    int i = 1;

    for (auto it = modes.begin(); it != modes.end(); ++it)
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
 * Sort the given table of messages.  Not the ideal place for this code.
 */
int l_CGlobalState_sort_messages(lua_State * l)
{
    /**
     * If the index.sort method is `none` then we do nothing.
     */
    CConfig *config = CConfig::instance();
    std::string method = config->get_string("index.sort", "none");


    /**
     * Build up the list of messages we were given.
     */
    if (!lua_istable(l, 2))
    {
        lua_pushnil(l);
        return 1;
    }

    std::vector<std::shared_ptr<CMessage> > *tmp = new CMessageList;

    lua_pushnil(l);

    while (lua_next(l, -2))
    {
        std::shared_ptr<CMessage> entry = l_CheckCMessage(l, -1);
        tmp->push_back(entry);
        lua_pop(l, 1);
    }

    /*
     * Now sort this vector.
     */
    if ( method != "none" )
        std::sort(tmp->begin(), tmp->end(), CMessage::compare);


    /*
     * Return to the caller.
     */
    lua_createtable(l, tmp->size(), 0);
    int i = 0;

    for (std::vector <std::shared_ptr<CMessage>>::iterator it = tmp->begin();
            it != tmp->end(); ++it)
    {
        std::shared_ptr<CMessage> m = (*it);
        push_cmessage(l, m);
        lua_rawseti(l, -2, i + 1);
        i++;
    }

    return 1;
}



/**
 * Implementation of `Global:select_messages`.
 */
int l_CGlobalState_select_message(lua_State * l)
{
    std::shared_ptr<CMessage> foo = l_CheckCMessage(l, 2);

    CGlobalState *global = CGlobalState::instance();
    global->set_message(foo);
    return 0;
}


/**
 * Register the global `Global` object to the Lua environment,
 * and setup our public methods upon which the user may operate.
 */
void InitGlobalState(lua_State * l)
{
    luaL_Reg sFooRegs[] =
    {
        {"current_maildir", l_CGlobalState_current_maildir},
        {"current_message", l_CGlobalState_current_message},
        {"current_messages", l_CGlobalState_current_messages},
        {"maildirs", l_CGlobalState_maildirs},
        {"modes", l_CGlobalState_modes},
        {"select_maildir", l_CGlobalState_select_maildir},
        {"select_message", l_CGlobalState_select_message},
        {"sort_messages", l_CGlobalState_sort_messages},
        {NULL, NULL}
    };
    luaL_newmetatable(l, "luaL_CGlobalState");

#if LUA_VERSION_NUM == 501
    luaL_register(l, NULL, sFooRegs);
#elif LUA_VERSION_NUM == 502 || LUA_VERSION_NUM == 503
    luaL_setfuncs(l, sFooRegs, 0);
#else
#error We are only tested under Lua 5.1, 5.2, or 5.3.
#endif

    lua_pushvalue(l, -1);
    lua_setfield(l, -1, "__index");
    lua_setglobal(l, "Global");

}
