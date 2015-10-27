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


#include <iostream>
#include <cursesw.h>


#include "config.h"
#include "global_state.h"
#include "lua.h"
#include "message.h"
#include "message_view.h"
#include "screen.h"

/**
 * Constructor.  NOP.
 */
CMessageView::CMessageView()
{
}

/**
 * Destructor.  NOP.
 */
CMessageView::~CMessageView()
{
}



/**
 * Return an array of lines of the message we're to draw,
 * via the `Message.to_string()` lua function.
 */
std::vector<std::string> CMessageView::get_message(std::shared_ptr<CMessage> msg)
{
    std::vector<std::string> results;


    /**
     * Get access to our lua-magic.
     */
    CLua *lua = CLua::instance();
    lua_State * l = lua->state();

    /**
     * Push a new Message object to the lua-stack, which relates to
     * this message.
     *
     * We do this so that we can call "to_string" on the Message object
     * and use that for display.
     */
    lua_getglobal(l, "Message");
    lua_getfield(l, -1, "to_string");

    //
    // This is buggy because the message is freed.
    //
    //  CMessage **udata = (CMessage **) lua_newuserdata(l, sizeof(CMessage *));
    //  *udata = message
    //
    // We can fix it temporarily by re-creating the current-message, thusly:
    //
    //  *udate = new CMessage( message->path() );
    //
    // TODO: Fix this properly
    //
    CMessage **udata = (CMessage **) lua_newuserdata(l, sizeof(CMessage *));
    *udata = new CMessage(msg->path());
    luaL_getmetatable(l, "luaL_CMessage");
    lua_setmetatable(l, -2);


    /**
     * Now call "to_string"
     */
    if (lua_pcall(l, 1, 1, 0) != 0)
    {
        std::cerr << "Error calling CMessage:to_string - " << lua_tostring(l, -1);
        return results;
    }

    /**
     * Now get the table we expected.
     */
    if (lua_istable(l, 1))
    {
        lua_pushnil(l);

        while (lua_next(l, -2))
        {
            const char *entry = lua_tostring(l, -1);
            results.push_back(entry);
            lua_pop(l, 1);
        }
    }

    CConfig *config = CConfig::instance();
    config->set("message.max", std::to_string(results.size()));

    return (results);
}



/**
 * This is the virtual function which is called to refresh the display
 * when the global.mode == "index"
 */
void CMessageView::draw()
{
    CScreen *screen = CScreen::instance();

    /**
     * Get the current message.
     */
    CGlobalState *state = CGlobalState::instance();
    std::shared_ptr<CMessage> message = state->current_message();

    /**
     * If there is no current message then we're done.
     */
    if (!message)
    {
        mvprintw(10, 10, "This is 'message' mode.");
        mvprintw(12, 10, "No message is currently selected.");
        return;
    }


    /**
     * Get the lines of the message, as an array of lines, such that
     * we can draw it.
     */
    std::vector<std::string> lines = get_message(message);


    /**
     * Get the size.
     */
    CConfig *config = CConfig::instance();
    std::string max_line = config->get_string("message.max");

    if (max_line.empty())
        max_line = "0";

    /**
     * Get the item under the cursor.
     */
    std::string current = config->get_string("message.current");

    if (current.empty())
    {
        config->set("message.current", "0" , false);
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
        config->set("message.current", "0" , false);
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

        if ((mailIndex < count) && (mailIndex < (int)lines.size()))
            buf = lines.at(mailIndex);

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
         * Change to the specified colour.
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
void CMessageView::on_idle()
{
}
