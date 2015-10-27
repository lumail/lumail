/**
 * index_view.cc - Draw a list of messages for an index.
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
#include "lua.h"
#include "index_view.h"
#include "global_state.h"
#include "screen.h"


/**
 * Constructor.  NOP.
 */
CIndexView::CIndexView()
{
}

/**
 * Destructor.  NOP.
 */
CIndexView::~CIndexView()
{
}


/**
 * Call Message.to_index()
 */
std::string CIndexView::format(std::shared_ptr<CMessage> cur)
{
    /**
     * Get access to our lua-magic.
     */
    CLua *lua = CLua::instance();
    lua_State * l = lua->state();


    /**
     * Push a new Message object to the lua-stack, which relates to
     * this message.
     *
     * We do this so that we can call "to_index" on the Message object
     * and use that for display.
     */
    lua_getglobal(l, "Message");
    lua_getfield(l, -1, "to_index");

    //
    // This is buggy because the message is freed.
    //
    //  CMessage **udata = (CMessage **) lua_newuserdata(l, sizeof(CMessage *));
    //  *udata = m.get()
    //
    // We can fix it temporarily by re-creating the current-message, thusly:
    //
    //  *udate = new CMessage( m->path() );
    //
    // TODO: Fix this properly - we need to use a shared_ptr for the
    // maildir_object.
    //
    CMessage **udata = (CMessage **) lua_newuserdata(l, sizeof(CMessage *));
    *udata = new CMessage(cur->path());
    luaL_getmetatable(l, "luaL_CMessage");
    lua_setmetatable(l, -2);


    /**
     * Now call "to_string"
     */
    if (lua_pcall(l, 1, 1, 0) != 0)
    {
        std::cerr << "Error calling CMessage:to_index - " << lua_tostring(l, -1);
        return "";
    }

    /**
     * Fingers crossed we now have the message.
     */
    if (lua_tostring(l, -1) == NULL)
    {
        std::cerr << "NULL OUTPUT!!!1!! " << std::endl;
        return "";
    }

    std::string output = lua_tostring(l, -1);
    return (output);
}


/**
 * This is the virtual function which is called to refresh the display
 * when the global.mode == "index"
 */
void CIndexView::draw()
{
    CScreen *screen = CScreen::instance();

    CGlobalState *state = CGlobalState::instance();
    std::shared_ptr<CMaildir> folder = state->current_maildir();

    if (!folder)
    {
        mvprintw(10, 10, "This is 'index' mode");
        mvprintw(12, 10, "This should draw a list of messages");
        mvprintw(14, 10, "No Maildir selected");
        return;
    }

    /**
     * Get the global-state.
     */
    CGlobalState *global = CGlobalState::instance();

    /**
     * Get the message-list.
     */
    CMessageList *messages = global->get_messages();

    /**
     * If empty we're done.
     */
    if (messages->size() < 1)
        return;


    /**
     * Draw them
     */
    CConfig *config = CConfig::instance();
    std::string max = config->get_string("index.max");

    if (max.empty())
        max = "0";

    /**
     * Get the item under the cursor.
     */
    std::string current = config->get_string("index.current");

    if (current.empty())
    {
        config->set("index.current", "0" , false);
        current = "0";
    }

    /**
     * Now we have:
     *
     *  max   -> The max number of messages.
     *  cur   -> The current message.
     * height -> The screen height.
     */
    std::string::size_type sz;
    size_t max_message = std::stoi(max, &sz);
    size_t cur_message = std::stoi(current, &sz);
    int    height      = CScreen::height();

    /**
     * Ensure we highlight the correct line.
     */
    if (cur_message > max_message)
    {
        config->set("index.current", "0" , false);
        cur_message = 0;
    }

    int middle = (height) / 2;
    int rowToHighlight = 0;
    vectorPosition topBottomOrMiddle = NONE;

    // TODO - Remove
    int count = max_message;
    int selected = cur_message;

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


    /**
     * OK so we have (at least one) selected maildir and we have messages.
     */
    int row = 0;

    for (row = 0; row < height; row++)
    {
        move(row, 0);

        /**
         * The current object.
         */
        std::shared_ptr<CMessage> msg = NULL;
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

        if ((mailIndex < count) && (mailIndex < (int)messages->size()))
            msg = messages->at(mailIndex);

        if (! msg)
            continue;

        //
        // TODO fix this
        //
        if (row == rowToHighlight)
            wattron(stdscr, A_REVERSE | A_STANDOUT);
        else
            wattroff(stdscr, A_REVERSE | A_STANDOUT);

        std::string buf;

        if (msg != NULL)
            buf = format(msg);

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
void CIndexView::on_idle()
{
}
