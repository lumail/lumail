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
#include "message_lua.h"
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
    push_cmessage(l, cur);

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
     * Get the message-list.
     */
    CGlobalState *global = CGlobalState::instance();
    CMessageList *messages = global->get_messages();

    /**
     * If empty we're done.
     */
    if (messages->size() < 1)
        return;


    /**
     * Build up an array of lines to display.
     */
    std::vector<std::string> display;

    for (std::vector<std::shared_ptr<CMessage>>::iterator it = messages->begin(); it != messages->end(); it++)
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
    std::string current = config->get_string("index.current");
    std::string max_line = config->get_string("index.max");

    if (max_line.empty())
    {
        config->set("index.max", "0", false);
        max_line = "0";
    }

    if (current.empty())
    {
        config->set("index.current", "0" , false);
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
        config->set("index.current", std::to_string(max) , false);
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
void CIndexView::on_idle()
{
}
