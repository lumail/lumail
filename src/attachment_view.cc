/*
 * attachment_vew.cc - Draw a list of attachments
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
#include "attachment_view.h"
#include "screen.h"


/*
 * Ensure we're registered as a valid view mode.
 */
REGISTER_VIEW_MODE(attachment, CAttachmentView)


/*
 * Constructor.
 */
CAttachmentView::CAttachmentView()
{
}

/*
 * Destructor.
 */
CAttachmentView::~CAttachmentView()
{
}



/*
 * Get the output of calling `attachment_view`, which is the text we'll display.
 */
std::vector<std::string> CAttachmentView::get_text()
{
    /*
     * Call the view-function.
     */
    CLua *lua = CLua::instance();
    std::vector<std::string> result = lua->function2table("attachment_view");

    /*
     * Store the number of lines we've retrieved.
     */
    CConfig *config = CConfig::instance();
    config->set("attachment.max", result.size());

    return (result);

}


/*
 * This is the virtual function which is called to refresh the display
 * when the global.mode == "attachment".
 */
void CAttachmentView::draw()
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
        mvprintw(10, 10, "This is 'attachment' mode.");
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
    int cur = config->get_integer("attachment.current");
    int max = config->get_integer("attachment.max");

    /*
     * Ensure we highlight the correct line.
     */
    if (cur > max)
    {
        config->set("attachment.current", max , false);
        cur = max;
    }

    /*
     * Draw the text, via our base-class.
     */
    CScreen *screen = CScreen::instance();
    screen->draw_text_lines(lines, cur, max);
}

/*
 * Called when things are idle.  NOP.
 */
void CAttachmentView::on_idle()
{
}
