/*
 * lua_view.cc - Draw the screen in lua-mode.
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


#include <cursesw.h>

#include "config.h"
#include "lua.h"
#include "lua_view.h"
#include "screen.h"


/**
 * NOP.
 */
CLuaView::CLuaView()
{
}


/**
 * NOP.
 */
CLuaView::~CLuaView()
{
}


/**
 * Get the output of calling `lua_view`, which is the text we'll display.
 */
std::vector<std::string> CLuaView::get_text()
{
    std::vector<std::string> result;

    /**
     * Get the lua state.
     */
    CLua *lua = CLua::instance();
    lua_State * l = lua->state();

    /**
     * If there is a lua_mode() function, then call it.
     */
    lua_getglobal(l, "lua_view");

    if (lua_isnil(l, -1))
        return (result);

    /**
     * Call the function.
     */
    lua_pcall(l, 0, 1, 0);

    /**
     * Now get the table we expected.
     */
    if (lua_istable(l, 1))
    {
        lua_pushnil(l);

        while (lua_next(l, -2))
        {
            const char *entry = lua_tostring(l, -1);
            result.push_back(entry);
            lua_pop(l, 1);
        }
    }

    /**
     * Store the number of lines we've retrieved.
     */
    CConfig *config = CConfig::instance();
    config->set("lua.max", result.size());

    return (result);

}


/**
 * This is the virtual function which is called to refresh the display
 * when the global.mode == "lua"
 */
void CLuaView::draw()
{
    /*
     * Get the string(s) we're supposed to display.
     */
    std::vector<std::string> txt = get_text();

    if (txt.empty())
    {
        mvprintw(10, 10, "This is 'lua' mode - define `lua_mode()' to setup output.");
        return;
    }


    CConfig *config = CConfig::instance();

    /*
     * Get the currently-selected item, and the size of the lines.
     */
    int cur = config->get_integer("lua.current");
    int max = config->get_integer("lua.max");


    /*
     * Ensure we highlight the correct line.
     */
    if (cur > max)
    {
        config->set("lua.current", max , false);
        cur = max;
    }

    /*
     * Draw the text, via our base-class.
     */
    CScreen *screen = CScreen::instance();
    screen->draw_text_lines(txt, cur, max, true);
}


/**
 * Called when things are idle.  NOP.
 */
void CLuaView::on_idle()
{
}
