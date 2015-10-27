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
 * Get the output of calling `lua_mode`
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

    return (result);

}


/**
 * This is the virtual function which is called to refresh the display
 * when the global.mode == "lua"
 */
void CLuaView::draw()
{
    CScreen *screen = CScreen::instance();
    screen->clear();

    std::vector<std::string> txt = get_text();

    if (txt.empty())
    {
        mvprintw(10, 10, "Hello World - This is 'lua' mode");
        return;
    }

    /**
     * Otherwise draw the text
     */
    int row = 0;

    for (std::vector < std::string >::iterator it = txt.begin(); it != txt.end(); ++it)
    {
        std::string buf = (*it);

        /**
         * Look for a colour-string
         */
        std::string colour = "";

        if (buf.at(0) == '$')
        {
            std::size_t start = buf.find("[");
            std::size_t end   = buf.find("]");

            if ((start != std::string::npos) &&
                    (end != std::string::npos))
            {
                colour = buf.substr(start + 1, end - start - 1);
                buf    = buf.substr(end + 1);
            }
        }

        /**
         * Ensure we draw a complete line - so that we cover
         * any old text.
         */
        while ((int)buf.length() < CScreen::width())
            buf += " ";

        /**
         * Ensure the line isn't too long, so we don't
         * wrap around.
         */
        if ((int)buf.length() >  CScreen::width())
            buf = buf.substr(0, CScreen::width() - 1);

        /**
         * Change to the colour in `colour`.
         */
        if (!colour.empty())
            wattron(stdscr, COLOR_PAIR(screen->get_colour(colour)));

        /**
         * Draw the line.
         */
        mvprintw(row, 0 , "%s", buf.c_str());

        /**
         * Reset the output.
         */
        if (! colour.empty())
            wattron(stdscr, COLOR_PAIR(screen->get_colour("white")));

        row += 1;
    }
}


/**
 * Called when things are idle.  NOP.
 */
void CLuaView::on_idle()
{
}
