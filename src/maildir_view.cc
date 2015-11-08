/*
 * maildir_view.cc - Draw a list of Maildirs.
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
#include "maildir.h"
#include "maildir_lua.h"
#include "maildir_view.h"


/**
 * Constructor.
 */
CMaildirView::CMaildirView()
{
}

/**
 * Destructor.
 */
CMaildirView::~CMaildirView()
{
}



/**
 * Get the output of calling the Lua function `maildir_view`, which is the text we'll display.
 *
 * If this function doesn't exist we'll draw nothing.
 *
 */
std::vector<std::string> CMaildirView::get_text()
{
    std::vector<std::string> result;

    /**
     * Get the lua state.
     */
    CLua *lua = CLua::instance();
    lua_State * l = lua->state();

    /*
     * If there is a maildir_view() function, then call it.
     */
    lua_getglobal(l, "maildir_view");

    if (lua_isnil(l, -1))
        return (result);

    /*
     * Call the function.
     */
    lua_pcall(l, 0, 1, 0);

    /*
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

    /*
     * Store the number of lines we've retrieved.
     */
    CConfig *config = CConfig::instance();
    int max = result.size();

    config->set("maildir.max", std::to_string(max));

    return (result);

}


/**
 * This is the virtual function which is called to refresh the display
 * when the global.mode == "maildir".
 */
void CMaildirView::draw()
{
    /*
     * Get all maildirs.
     */
    CGlobalState *state = CGlobalState::instance();
    std::vector<std::shared_ptr<CMaildir> > maildirs = state->get_maildirs();

    /*
     * If there is nothing present then we're done.
     */
    if (maildirs.size() < 1)
    {
        mvprintw(10, 10, "This is 'maildir' mode.");
        mvprintw(12, 10, "No maildirs are currently visible.");
        return;
    }

    /*
     * Get the lines of the message, as an array of lines, such that
     * we can draw it.
     */
    std::vector<std::string> lines = get_text();

    /*
     * Now handle our offsets/etc.
     */
    CConfig *config = CConfig::instance();

    /*
     * Get the currently-selected item, and the size of the lines.
     */
    std::string current = config->get_string("maildir.current");
    std::string max_line = config->get_string("maildir.max");

    if (max_line.empty())
    {
        config->set("maildir.max", "0", false);
        max_line = "0";
    }

    if (current.empty())
    {
        config->set("maildir.current", "0" , false);
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
        config->set("maildir.current", std::to_string(max) , false);
        cur = max;
    }

    /*
     * Draw the text, via our base-class.
     */
    CScreen *screen = CScreen::instance();
    screen->draw_text_lines(lines, cur, max);

}

/**
 * Called when things are idle.
 */
void CMaildirView::on_idle()
{
}
