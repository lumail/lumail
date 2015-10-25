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

#include "global_state.h"
#include "lua.h"

#include "message.h"
#include "message_view.h"
#include "util.h"

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
 * This is the virtual function which is called to refresh the display
 * when the global.mode == "index"
 */
void CMessageView::draw()
{

    /**
     * Get the current message.
     */
    CGlobalState *state = CGlobalState::instance();
    CMessage *message   = state->current_message();

    /**
     * If there is no current message then we're done.
     */
    if (message == NULL)
    {
        mvprintw(10, 10, "This is 'message' mode.");
        mvprintw(12, 10, "No message is currently selected.");
        return;
    }

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
    *udata = new CMessage( message->path() );
    luaL_getmetatable(l, "luaL_CMessage");
    lua_setmetatable(l, -2);


    /**
     * Now call "to_string"
     */
    if (lua_pcall(l, 1, 1, 0) != 0)
    {
        std::cerr << "Error calling CMessage:to_string - " << lua_tostring(l, -1);
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
     * Split the message into an array.
     */
    std::vector<std::string> lines = CUtil::split(output, '\n');

    /**
     * Draw it.
     */
    int cur = 0;
    int max = 40 ; // CScreen::height();

    while ((cur < max) && (cur < (int)lines.size()))
    {
        mvprintw(cur, 0, "%s", lines.at(cur).c_str());
        cur += 1;
    }


}
