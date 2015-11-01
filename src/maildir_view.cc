/**
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
 * Constructor.  NOP.
 */
CMaildirView::CMaildirView()
{
}

/**
 * Destructor.  NOP.
 */
CMaildirView::~CMaildirView()
{
}

/**
 * This is the virtual function which is called to refresh the display
 * when the global.mode == "maildir"
 */
void CMaildirView::draw()
{
    /**
     * Get all maildirs.
     */
    CGlobalState *state = CGlobalState::instance();
    std::vector<std::shared_ptr<CMaildir> > maildirs = state->get_maildirs();

    /**
     * For each maildir format it.
     */
    std::vector<std::string> display;

    for (std::vector < std::shared_ptr<CMaildir> >::iterator it = maildirs.begin(); it != maildirs.end(); it++)
    {
        display.push_back(format(*(it)));
    }


    /**
     * Now handle our offsets/etc.
     */
    CConfig *config = CConfig::instance();

    /**
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
        config->set("maildir.current", std::to_string(max) , false);
        cur = max;
    }

    /**
     * Draw the text, via our base-class.
     */
    CScreen *screen = CScreen::instance();
    screen->draw_text_lines(display, cur, max);

}

/**
 * Called when things are idle.  NOP.
 */
void CMaildirView::on_idle()
{
}

/**
 * Call Maildir.to_string() against the maildir.
 */
std::string CMaildirView::format(std::shared_ptr<CMaildir> cur)
{
    /**
     * Get access to our lua-magic.
     */
    CLua *lua = CLua::instance();
    lua_State * l = lua->state();

    /**
     * The function we're going to call...
     */
    lua_getglobal(l, "Maildir");
    lua_getfield(l, -1, "to_string");

    /**
     * Push the Maildir object to the lua-stack, which relates to
     * this message.
     */
    push_cmaildir(l, cur);


    /**
     * Now call "to_string"
     */
    if (lua_pcall(l, 1, 1, 0) != 0)
    {
        std::cerr << "Error calling CMaildir:to_string - " << lua_tostring(l, -1);
        return "";
    }

    /**
     * Fingers crossed we now have output.
     */
    if (lua_tostring(l, -1) == NULL)
    {
        std::cerr << "NULL OUTPUT!!!1!! " << std::endl;
        return "";
    }

    std::string output = lua_tostring(l, -1);
    return output;
}
