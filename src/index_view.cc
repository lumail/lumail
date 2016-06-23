/*
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
#include "global_state.h"
#include "index_view.h"
#include "lua.h"
#include "message_lua.h"
#include "screen.h"


/*
 * Constructor.
 */
CIndexView::CIndexView()
{
}

/*
 * Destructor.
 */
CIndexView::~CIndexView()
{
}



/*
 * Get the output of calling `index_view`, which is the text we'll display.
 */
std::vector<std::string> CIndexView::get_text()
{
    /*
     * Call the view-function.
     */
    CLua *lua = CLua::instance();
    std::vector<std::string> result = lua->function2table("index_view");

    /*
     * Store the number of lines we've retrieved.
     */
    CConfig *config = CConfig::instance();
    config->set("index.max", result.size());

    return (result);

}

/*
 * This is the virtual function which is called to refresh the display
 * when the global.mode == "index".
 */
void CIndexView::draw()
{
    CGlobalState *state = CGlobalState::instance();
    std::shared_ptr<CMaildir> folder = state->current_maildir();

    if (!folder)
    {
        mvprintw(10, 10, "This is 'index' mode");
        mvprintw(12, 10, "This should draw a list of messages, but there is no selected Maildir.");
        return;
    }

    /*
     * Get the formatted list of text to draw.
     */
    std::vector<std::string> display = get_text();

    /*
     * Now handle our offsets/etc.
     */
    CConfig *config = CConfig::instance();

    /*
     * Get the currently-selected item, and the size of the lines.
     */
    int cur = config->get_integer("index.current");
    int max = config->get_integer("index.max");


    /*
     * Ensure we highlight the correct line.
     */
    if (cur > max)
    {
        if (max > 0)
            cur = max - 1;
        else
            cur = 0;

        config->set("index.current", cur, false);
    }

    /*
     * Draw the text, via our base-class.
     */
    CScreen *screen = CScreen::instance();
    screen->draw_text_lines(display, cur, max);
}


/*
 * Called when things are idle.
 */
void CIndexView::on_idle()
{
}
