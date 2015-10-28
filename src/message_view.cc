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

    CConfig *config = CConfig::instance();

    /**
     * Get the currently-selected item, and the size of the lines.
     */
    std::string current = config->get_string("message.current");
    std::string max_line = config->get_string("message.max");

    if (max_line.empty())
    {
        config->set("message.max", "0", false);
        max_line = "0";
    }

    if (current.empty())
    {
        config->set("message.current", "0" , false);
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
        config->set("message.current", std::to_string(max) , false);
        cur = max;
    }

    /**
     * Draw the text, via our base-class.
     */
    CScreen *screen = CScreen::instance();
    screen->draw_text_lines(lines, cur, max, true);
}

/**
 * Called when things are idle.  NOP.
 */
void CMessageView::on_idle()
{
}
