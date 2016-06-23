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


/*
 * Constructor.
 */
CMaildirView::CMaildirView()
{
}

/*
 * Destructor.
 */
CMaildirView::~CMaildirView()
{
}



/*
 * Get the output of calling the Lua function `maildir_view`, which is the text we'll display.
 *
 * If this function doesn't exist we'll draw nothing.
 *
 */
std::vector<std::string> CMaildirView::get_text()
{
    /*
     * Call the view-function.
     */
    CLua *lua = CLua::instance();
    std::vector<std::string> result = lua->function2table("maildir_view");

    /*
     * Store the number of lines we've retrieved.
     */
    CConfig *config = CConfig::instance();
    config->set("maildir.max", result.size());

    return (result);

}


/*
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
    int cur = config->get_integer("maildir.current");
    int max = config->get_integer("maildir.max");

    /*
     * Ensure we highlight the correct line.
     */
    /*
     * Ensure we highlight the correct line.
     */
    if (cur > max)
    {
        if (max > 0)
            cur = max - 1;
        else
            cur = 0;

        config->set("maildir.current", cur, false);
    }

    /*
     * Draw the text, via our base-class.
     */
    CScreen *screen = CScreen::instance();
    screen->draw_text_lines(lines, cur, max);

}

/*
 * Called when things are idle.
 */
void CMaildirView::on_idle()
{
}
