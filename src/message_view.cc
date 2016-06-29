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

/*
 * Ensure we're registered as a valid view mode.
 */
REGISTER_VIEW_MODE(message, CMessageView)


/*
 * Constructor
 */
CMessageView::CMessageView()
{
}

/*
 * Destructor.
 */
CMessageView::~CMessageView()
{
}



/*
 * Get the output of calling `message_view`, which is the text we'll display.
 *
 * The `message_view` Lua function should return a table of arrays to dipslay
 * having formatted the message as it sees fit.
 */
std::vector<std::string> CMessageView::get_text()
{
    /*
     * Call the view-function.
     */
    CLua *lua = CLua::instance();
    std::vector<std::string> result = lua->function2table("message_view");

    /*
     * Store the number of lines we've retrieved.
     */
    CConfig *config = CConfig::instance();
    config->set("message.max", result.size());

    return (result);

}


/*
 * This is the virtual function which is called to refresh the display
 * when the global.mode == "message".
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
    int cur = config->get_integer("message.current");
    int max = config->get_integer("message.max");

    /*
     * Ensure we highlight the correct line.
     */
    if (cur > max)
    {
        config->set("message.current", max, false);
        cur = max;
    }

    /*
     * Draw the text, via our base-class.
     */
    CScreen *screen = CScreen::instance();
    screen->draw_text_lines(lines, cur, max, true);
}

/*
 * Called when things are idle.
 */
void CMessageView::on_idle()
{
}
