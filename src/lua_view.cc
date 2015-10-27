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

    /**
     * Store the number of lines we've retrieved.
     */
    CConfig *config = CConfig::instance();
    config->set("lua.max", std::to_string(result.size()));

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
        mvprintw(10, 10, "This is 'lua' mode - define `lua_mode()' to setup output.");
        return;
    }


    /**
     * Get the size.
     */
    CConfig *config = CConfig::instance();
    std::string max_line = config->get_string("lua.max");

    if (max_line.empty())
        max_line = "0";

    /**
     * Get the item under the cursor.
     */
    std::string current = config->get_string("lua.current");

    if (current.empty())
    {
        config->set("lua.current", "0" , false);
        current = "0";
    }

    /**
     * Now we have:
     *
     *  max   -> The max number of lines to display.
     *  cur   -> The current line.
     * height -> The screen height.
     */
    std::string::size_type sz;
    size_t max = std::stoi(max_line, &sz);
    size_t cur = std::stoi(current, &sz);
    int height = CScreen::height();


    /**
     * Ensure we highlight the correct line.
     */
    if (cur > max)
    {
        config->set("lua.current", "0" , false);
        cur = 0;
    }

    int middle = (height) / 2;
    int rowToHighlight = 0;
    vectorPosition topBottomOrMiddle = NONE;

    // TODO - Remove
    int count = max;
    int selected = cur;

    /**
     * default to TOP if our list is shorter then the screen height
     */
    if (selected < middle || count <= height)
    {
        topBottomOrMiddle = TOP;
        rowToHighlight = selected;
        /**
         * if height is uneven we have to switch to the BOTTOM case on row earlier
         */
    }
    else if ((count - selected <= middle) || (height % 2 == 1 && count - selected <= middle + 1))
    {
        topBottomOrMiddle = BOTTOM;
        rowToHighlight =  height - count + selected - 1 ;
    }
    else
    {
        topBottomOrMiddle = MIDDLE;
        rowToHighlight = middle;
    }


    int row = 0;

    for (row = 0; row < height; row++)
    {
        move(row, 0);

        /**
         * The current object.
         */
        int mailIndex = count;

        if (topBottomOrMiddle == TOP)
        {
            /**
             * we start at the top of the list so just use row
             */
            mailIndex = row;
        }
        else if (topBottomOrMiddle == BOTTOM)
        {
            /**
             * when we reached the end of the list mailIndex can maximally be
             * count-1, that this is given can easily be shown
             * row:=height-2 -> count-height+row+1 = count-height+height-2+1 = count-1
             */
            mailIndex = count - height + row + 1;
        }
        else if (topBottomOrMiddle == MIDDLE)
        {
            mailIndex = row + selected - middle;
        }


        std::string buf;

        if ((mailIndex < count) && (mailIndex < (int)txt.size()))
            buf = txt.at(mailIndex);

        if (buf.empty())
            continue;


        if (row == rowToHighlight)
            wattron(stdscr, A_REVERSE | A_STANDOUT);
        else
            wattroff(stdscr, A_REVERSE | A_STANDOUT);

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
         * TODO: Change to the colour in `colour`.
         */
        if (!colour.empty())
            wattron(stdscr, COLOR_PAIR(screen->get_colour(colour)));

        printw("%s", buf.c_str());

        if (! colour.empty())
            wattron(stdscr, COLOR_PAIR(screen->get_colour("white")));
    }

    /**
     * Ensure we turn off the attribute on the last line - so that
     * any blank lines are "normal".
     */
    wattroff(stdscr, A_REVERSE | A_STANDOUT);
    wattron(stdscr, COLOR_PAIR(screen->get_colour("white")));
}


/**
 * Called when things are idle.  NOP.
 */
void CLuaView::on_idle()
{
}
