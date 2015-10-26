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
    int cur = 0;

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


    std::string::size_type sz;
    size_t max_message = std::stoi(max, &sz);
    size_t cur_message = std::stoi(current, &sz);

    if (max_message < 1)
    {
        mvprintw(10, 10, "This is 'index' mode");
        mvprintw(12, 10, "There are no messages found");
        return;
    }

    int drawn = 0;
    int height = CScreen::height();

    /**
     * Get access to our lua-magic.
     */
    CLua *lua = CLua::instance();
    lua_State * l = lua->state();

    /**
     * Draw each maildir in the list.
     */
    while (cur < (int)max_message)
    {
      /**
       * Don't draw over the top of the screen.
       */
      drawn += 1;
      if ( drawn >= height )
        break;


      /**
         * Get the message.
         */
        std::shared_ptr<CMessage>  m = messages->at(cur);


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
        *udata = new CMessage(m->path());
        luaL_getmetatable(l, "luaL_CMessage");
        lua_setmetatable(l, -2);


        /**
         * Now call "to_string"
         */
        if (lua_pcall(l, 1, 1, 0) != 0)
        {
            std::cerr << "Error calling CMessage:to_index - " << lua_tostring(l, -1);
            return;
        }

        /**
         * Fingers crossed we now have the message.
         */
        if (lua_tostring(l, -1) == NULL)
        {
            std::cerr << "NULL OUTPUT!!!1!! " << std::endl;
            return;
        }

        std::string output = lua_tostring(l, -1);


        /**
         * Look for a colour-string
         */
        std::string colour = "";

        if (output.at(0) == '$')
        {
            std::size_t start = output.find("[");
            std::size_t end   = output.find("]");

            if ((start != std::string::npos) &&
                    (end != std::string::npos))
            {
                colour = output.substr(start + 1, end - start - 1);
                output = output.substr(end + 1);
            }
        }

        /**
         * Ensure we draw a complete line - so that we cover
         * any old text.
         */
        while ((int)output.length() < CScreen::width())
            output += " ";

        /**
         * Ensure the line isn't too long, so we don't
         * wrap around.
         */
        if ( (int)output.length() >  CScreen::width() )
          output = output.substr(0, CScreen::width()-1);


        /**
         * TODO: Change to the colour in `colour`.
         */
        if (!colour.empty())
            wattron(stdscr, COLOR_PAIR(screen->get_colour(colour)));

        if (cur == (int)cur_message)
            wattron(stdscr, A_REVERSE | A_STANDOUT);
        else
            wattroff(stdscr, A_REVERSE | A_STANDOUT);

        mvprintw(cur, 0, "%s", output.c_str());

        /**
         * Reset the colour
         */
        if (! colour.empty())
            wattron(stdscr, COLOR_PAIR(screen->get_colour("white")));

        cur += 1;
    }

    /**
     * Ensure we turn off the attribute on the last line - so that
     * any blank lines are "normal".
     */
    wattroff(stdscr, A_REVERSE | A_STANDOUT);
}

/**
 * Called when things are idle.  NOP.
 */
void CIndexView::on_idle()
{
}
