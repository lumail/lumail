/*
 * panel_lua.cc - Export the `Panel` object to Lua.
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

#include "screen.h"


/**
 * Append a line of text to that being displayed.
 */
int l_CPanel_append(lua_State * l)
{
    const char *str = lua_tostring(l, 2);

    CScreen *screen = CScreen::instance();
    screen->status_panel_append(str);
    return 0;
}


/**
 * Get/set the height of the panel.
 */
int l_CPanel_height(lua_State * l)
{
    int new_height = lua_tointeger(l, 2);

    CScreen *screen = CScreen::instance();

    if (new_height)
    {
        screen->status_panel_height(new_height);
        return 0;
    }
    else
    {
        lua_pushinteger(l, screen->status_panel_height());
        return 1;
    }
}


/**
 * Hide the panel.
 */
int l_CPanel_hide(lua_State * l)
{
    CScreen *screen = CScreen::instance();
    screen->status_panel_hide();
    return 0;
}


/**
 * Show the panel.
 */
int l_CPanel_show(lua_State * l)
{
    CScreen *screen = CScreen::instance();
    screen->status_panel_show();
    return 0;
}


/**
 * Get the array of text which is displayed.
 */
int l_CPanel_text(lua_State * l)
{
    CScreen *screen = CScreen::instance();
    std::vector < std::string > cur = screen->status_panel_text();

    lua_newtable(l);
    int i = 1;

    for (auto it = cur.begin(); it != cur.end(); ++it)
    {
        std::string line = (*it);

        lua_pushinteger(l, i);
        lua_pushstring(l, line.c_str());

        lua_settable(l, -3);
        i += 1;
    }

    return 1;
}


/**
 * Get/Set the title text which is displayed.
 */
int l_CPanel_title(lua_State * l)
{
    CScreen *screen = CScreen::instance();

    const char *str = lua_tostring(l, 2);

    if (str != NULL)
    {
        screen->status_panel_title(str);
        return 0;
    }
    else
    {
        std::string existing = screen->status_panel_title();
        lua_pushstring(l, existing.c_str());
        return 1;
    }
}


/**
 * Toggle the visibility of the panel.
 */
int l_CPanel_toggle(lua_State * l)
{
    CScreen *screen = CScreen::instance();
    screen->status_panel_toggle();
    return 0;
}


/**
 * Is the panel visible?
 */
int l_CPanel_visible(lua_State * l)
{
    CScreen *screen = CScreen::instance();

    if (screen->status_panel_visible())
        lua_pushboolean(l, 1);
    else
        lua_pushboolean(l, 0);

    return 1;
}


/**
 * Export the `Panel` class to Lua.
 *
 * Bind the appropriate methods to that object.
 */
void InitPanel(lua_State * l)
{
    luaL_Reg sFooRegs[] =
    {
        {"append",  l_CPanel_append},
        {"height",  l_CPanel_height},
        {"hide",    l_CPanel_hide},
        {"show",    l_CPanel_show},
        {"text",    l_CPanel_text},
        {"title",   l_CPanel_title},
        {"toggle",  l_CPanel_toggle},
        {"visible", l_CPanel_visible},
        {NULL,      NULL}
    };
    luaL_newmetatable(l, "luaL_CPanel");

#if LUA_VERSION_NUM == 501
    luaL_register(l, NULL, sFooRegs);
#elif LUA_VERSION_NUM == 502
    luaL_setfuncs(l, sFooRegs, 0);
#else
#error unsupported Lua version
#endif

    lua_pushvalue(l, -1);
    lua_setfield(l, -1, "__index");
    lua_setglobal(l, "Panel");
}
