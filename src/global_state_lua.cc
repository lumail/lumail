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


extern "C"
{
#include <lua.h>
#include <lauxlib.h>
#include <lualib.h>
}



#include "global_state.h"
#include "maildir_lua.h"



/**
 * Get all maildirs
 */
int l_CGlobalState_maildirs(lua_State * L)
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
 * Get the currently-selected maildir
 */
int l_CGlobalState_current_maildir(lua_State * L)
{
    CGlobalState *global = CGlobalState::instance();
    std::shared_ptr<CMaildir> current = global->current_maildir();

    if (current)
        push_cmaildir(L, current);
    else
        lua_pushnil(L);

    return 1;
}


void InitGlobalState(lua_State * l)
{
    luaL_Reg sFooRegs[] =
    {
        {"maildirs", l_CGlobalState_maildirs},
        {"current_maildir", l_CGlobalState_current_maildir},
        {NULL, NULL}
    };
    luaL_newmetatable(l, "luaL_CGlobalState");

#if LUA_VERSION_NUM == 501
    luaL_register(l, NULL, sFooRegs);
#elif LUA_VERSION_NUM == 502
    luaL_setfuncs(l, sFooRegs, 0);
#else
#error unsupported Lua version
#endif

    lua_pushvalue(l, -1);
    lua_setfield(l, -1, "__index");
    lua_setglobal(l, "Global");

}
