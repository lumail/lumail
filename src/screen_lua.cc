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
#include "input_queue.h"
#include "maildir_lua.h"
#include "message_lua.h"
#include "screen.h"



/**
 * @file screen_lua.cc
 *
 * This file implements the trivial exporting of our screen-related
 * functions implemented in C++, to Lua.  Lua-usage looks something
 * like this:
 *
 *<code>
 *   -- Clear the screen  <br/>
 *   Screen:clear()<br/>
 *</code>
 *
 */


/**
 * Implementation of Screen:clear().
 */
int l_CScreen_clear(lua_State * l)
{
    (void)l;

    CScreen *foo = CScreen::instance();
    foo->clear();
    return 0;
}


/**
 * Implementation of Screen:draw().
 */
int l_CScreen_draw(lua_State * l)
{
    int x = luaL_checkinteger(l, 2);
    int y = luaL_checkinteger(l, 3);
    const char *txt = luaL_checkstring(l, 4);

    mvprintw(x, y, txt);
    return 0;
}

/**
 * Implementation of Screen:execute().
 */
int l_CScreen_execute(lua_State *l)
{
    (void)l;

    const char *prog = luaL_checkstring(l, 2);

    CScreen *foo = CScreen::instance();
    foo->execute(prog);
    return 0;


}


/**
 * Implementation of Screen:exit().
 */
int l_CScreen_exit(lua_State * l)
{
    (void)l;

    CScreen *foo = CScreen::instance();
    foo->exit_main_loop();
    return 0;
}


/**
 * Implementation of Screen:get_line().
 *
 * TAB-completion is supported via the lua `on_complete` callback,
 * and history is maintained via CHistory.
 */
int l_CScreen_get_line(lua_State * l)
{
    /*
     * Get the prompt?
     */
    const char *prompt = luaL_checkstring(l, 2);
    std::string input = "";

    if (lua_gettop(l) > 2)
        input = luaL_checkstring(l, 3);

    CScreen *foo = CScreen::instance();
    std::string received = foo->get_line(prompt, input);

    if (received.empty())
        lua_pushnil(l);
    else
        lua_pushstring(l, received.c_str());

    return 1;
}


/**
 * Implementation of Screen:prompt_chars().
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
 * Implementation of Screen:get_char().
 */
int l_CScreen_get_char(lua_State * l)
{
    const char *prompt = luaL_checkstring(l, 2);

    CScreen *foo    = CScreen::instance();
    std::string out = foo->get_char(prompt);

    lua_pushstring(l, out.c_str());
    return 1;
}


/**
 * Implementation of Screen:height().
 */
int l_CScreen_height(lua_State * l)
{
    (void)l;
    CScreen *foo = CScreen::instance();
    lua_pushinteger(l, foo->height());
    return 1;
}

/**
 * Implementation of Screen:sleep().
 */
int l_CScreen_sleep(lua_State * l)
{
    const int delay = luaL_checkinteger(l, 2);
    CScreen *foo = CScreen::instance();
    foo->sleep(delay);
    return 0;
}


/**
 * Implementation of Screen:sleep().
 */
int l_CScreen_stuff(lua_State * l)
{
    const char *str = luaL_checkstring(l, 2);
    CInputQueue *input = CInputQueue::instance();
    input->add_input(str);
    return 0;
}

/**
 * Implementation of Screen:width().
 */
int l_CScreen_width(lua_State * l)
{
    (void)l;
    CScreen *foo = CScreen::instance();
    lua_pushinteger(l, foo->width());
    return 1;
}

/**
 * Register the global `Screen` object to the Lua environment, and
 * setup our public (static) methods upon which the user may operate.
 */
void InitScreen(lua_State * l)
{
    luaL_Reg sFooRegs[] =
    {
        {"clear", l_CScreen_clear},
        {"draw", l_CScreen_draw},
        {"execute",  l_CScreen_execute},
        {"exit",  l_CScreen_exit},
        {"get_char", l_CScreen_get_char},
        {"get_line", l_CScreen_get_line},
        {"height", l_CScreen_height},
        {"prompt", l_CScreen_prompt_chars},
        {"sleep", l_CScreen_sleep},
        {"stuff", l_CScreen_stuff},
        {"width", l_CScreen_width},
        {NULL, NULL}
    };
    luaL_newmetatable(l, "luaL_CScreen");

#if LUA_VERSION_NUM == 501
    luaL_register(l, NULL, sFooRegs);
#elif LUA_VERSION_NUM == 502 || LUA_VERSION_NUM == 503
    luaL_setfuncs(l, sFooRegs, 0);
#else
#error We are only tested under Lua 5.1, 5.2, or 5.3.
#endif

    lua_pushvalue(l, -1);
    lua_setfield(l, -1, "__index");
    lua_setglobal(l, "Screen");

}
