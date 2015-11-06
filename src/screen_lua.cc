/*
 * screen_lua.cc - Export our screen-object to Lua.
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
#include "message_lua.h"
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


int l_CScreen_execute(lua_State *l)
{
    const char *prog = luaL_checkstring(l, 2);

    CScreen *foo = CScreen::instance();
    foo->execute(prog);
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
 * Select a maildir, by index.
 */
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

/**
 * Select a message by index.
 */
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


/**
 * Read a line of input from the user, with history and TAB-completion
 */
int l_CScreen_get_line(lua_State * l)
{
    /**
     * Get the have a prompt?
     */
    const char *prompt = luaL_checkstring(l, 2);

    CScreen *foo = CScreen::instance();
    std::string received = foo->get_line(prompt);

    if (received.empty())
        lua_pushnil(l);
    else
        lua_pushstring(l, received.c_str());

    return 1;
}


/**
 * Show a message and return only a valid keypress from a given set.
 */
int l_CScreen_prompt_chars(lua_State * l)
{
    const char *prompt = luaL_checkstring(l, 2);
    const char *chars  = luaL_checkstring(l, 3);

    CScreen *foo = CScreen::instance();
    std::string out  = foo->prompt_chars(prompt, chars);

    lua_pushstring(l, out.c_str());

    return 1;
}

/**
 * Get the screen height.
 */
int l_CScreen_height(lua_State * l)
{
    CScreen *foo = CScreen::instance();
    lua_pushinteger(l, foo->height());
    return 1;
}

/**
 * Delay execution for the given period, in seconds.
 */
int l_CScreen_sleep(lua_State * l)
{
    const int delay = luaL_checkinteger(l, 2);
    CScreen *foo = CScreen::instance();
    foo->sleep(delay);
    return 0;
}

/**
 * Get the screen width.
 */
int l_CScreen_width(lua_State * l)
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
        {"execute",  l_CScreen_execute},
        {"exit",  l_CScreen_exit},
        {"get_line", l_CScreen_get_line},
        {"height", l_CScreen_height},
        {"prompt", l_CScreen_prompt_chars},
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
