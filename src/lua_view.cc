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


#include <cursesw.h>

#include "config.h"
#include "lua.h"
#include "lua_view.h"
#include "screen.h"


/**
 * Constructor.  NOP.
 */
CLuaView::CLuaView()
{
}


/**
 * Destructor.  NOP.
 */
CLuaView::~CLuaView()
{
}


/**
 * Get the output of calling `lua_mode`, which is the text we'll display.
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
    lua_getglobal(l, "lua_mode");

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
    int max = result.size();

    if (max > 0)
        max -= 1;

    config->set("lua.max", std::to_string(max));

    return (result);

}


/**
 * This is the virtual function which is called to refresh the display
 * when the global.mode == "lua"
 */
void CLuaView::draw()
{
    /**
     * Get the string(s) we're supposed to display.
     */
    std::vector<std::string> txt = get_text();

    if (txt.empty())
    {
        mvprintw(10, 10, "This is 'lua' mode - define `lua_mode()' to setup output.");
        return;
    }


    CConfig *config = CConfig::instance();

    /**
     * Get the currently-selected item, and the size of the lines.
     */
    std::string current = config->get_string("lua.current");
    std::string max_line = config->get_string("lua.max");

    if (max_line.empty())
    {
        config->set("lua.max", "0", false);
        max_line = "0";
    }

    if (current.empty())
    {
        config->set("lua.current", "0" , false);
        current = "0";
    }

    /**
     * Now we should have, as integers:
     *
     *  max   -> The max number of lines to display.
     *  cur   -> The current line.
     */
    std::string::size_type sz;
    size_t max = std::stoi(max_line, &sz);
    size_t cur = std::stoi(current, &sz);

    /**
     * Ensure we highlight the correct line.
     */
    if (cur > max)
    {
        config->set("lua.current", std::to_string(max) , false);
        cur = max;
    }

    /**
     * Draw the text, via our base-class.
     */
    CScreen *screen = CScreen::instance();
    screen->draw_text_lines(txt, cur, max);
}


/**
 * Called when things are idle.  NOP.
 */
void CLuaView::on_idle()
{
}
