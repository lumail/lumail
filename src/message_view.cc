/*
 * message_vew.cc - Draw the currently selected message, via Lua.
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
#include "message_lua.h"
#include "message_view.h"
#include "screen.h"

/**
 * NOP.
 */
CMessageView::CMessageView()
{
}

/**
 * NOP.
 */
CMessageView::~CMessageView()
{
}



/**
 * Get the output of calling `message_view`, which is the text we'll display.
 *
 * The `message_view` lua function should return a table of arrays to dipslay
 * having formatted the message as it sees fit.
 */
std::vector<std::string> CMessageView::get_text()
{
    std::vector<std::string> result;

    /**
     * Get the lua state.
     */
    CLua *lua = CLua::instance();
    lua_State * l = lua->state();

    /**
     * If there is a message_view() function, then call it.
     */
    lua_getglobal(l, "message_view");

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
    int max = result.size();

    if (max > 0)
        max -= 1;

    CConfig *config = CConfig::instance();
    config->set("message.max", std::to_string(max));

    return (result);

}


/**
 * This is the virtual function which is called to refresh the display
 * when the global.mode == "message"
 */
void CMessageView::draw()
{
    /*
     * Get the current message.
     */
    CGlobalState *state = CGlobalState::instance();
    std::shared_ptr<CMessage> message = state->current_message();

    /*
     * If there is no current message then we're done.
     */
    if (!message)
    {
        mvprintw(10, 10, "This is 'message' mode.");
        mvprintw(12, 10, "No message is currently selected.");
        return;
    }

    /*
     * Get the lines of the message, as an array of lines, such that
     * we can draw it.
     */
    std::vector<std::string> lines = get_text();

    CConfig *config = CConfig::instance();

    /*
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

    /*
     * Now we should have, as integers:
     *
     *  max   -> The max number of lines to display.
     *  cur   -> The current line.
     */
    std::string::size_type sz;
    size_t max = std::stoi(max_line, &sz);
    size_t cur = std::stoi(current, &sz);

    /*
     * Ensure we highlight the correct line.
     */
    if (cur > max)
    {
        config->set("message.current", std::to_string(max) , false);
        cur = max;
    }

    /*
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
