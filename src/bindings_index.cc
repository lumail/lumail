/**
 * bindings_index.cc - Bindings for all index-related Lua primitives.
 *
 * This file is part of lumail: http://lumail.org/
 *
 * Copyright (c) 2013 by Steve Kemp.  All rights reserved.
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
 *
 */


#include <algorithm>
#include <string>

#include "bindings.h"
#include "debug.h"
#include "global.h"
#include "message.h"



/**
 * Get the current offset into the message-list.
 */
int index_offset(lua_State *L)
{
    CGlobal *global = CGlobal::Instance();
    int offset = global->get_selected_message();

    lua_pushinteger(L, offset);
    return (1);
}


/**
 * Jump to the given message.
 */
int jump_index_to(lua_State * L)
{
    int offset = lua_tonumber(L, -1);

    CGlobal *global = CGlobal::Instance();
    global->set_selected_message(offset);

    /**
     * We've changed messages, so reset the current position.
     */
    global->set_message_offset(0);

    return (0);
}


/**
 * scroll down the message list.
 */
int scroll_index_down(lua_State * L)
{
    int step = lua_tonumber(L, -1);

    CGlobal *global = CGlobal::Instance();

    int cur = global->get_selected_message();
    cur += step;

    global->set_selected_message(cur);

    /**
     * We've changed messages, so reset the current position.
     */
    global->set_message_offset(0);

    return 0;
}


/**
 * Search for the next message matching the pattern.
 */
int scroll_index_to(lua_State * L)
{
    const char *str = lua_tostring(L, -1);

    if (str == NULL)
        return luaL_error(L, "Missing argument to scroll_index_to(..)");

    /**
     * Lower-case version of the string.
     */
    std::string *find = new std::string(str);
    std::transform(find->begin(), find->end(), find->begin(), tolower);

    /**
     * get the current messages
     */
    CGlobal *global = CGlobal::Instance();
    std::vector<CMessage *> *messages = global->get_messages();

    /**
     * If we have no messages we're not scrolling anywhere.
     */
    if ( messages == NULL )
    {
        free( find );
        return 0;
    }


    int max      = messages->size();
    int selected = global->get_selected_message();

    int i = selected + 1;

    while (i != selected)
    {
        if (i >= max)
            break;

        /**
         * Format the message, and lower-case it.
         */
        CMessage *cur = messages->at(i);

        /**
         * Now look for it.
         */
        if (cur->matches_filter( find ) )
        {
            global->set_selected_message(i);
            break;
        }
        i += 1;

        if (i >= max)
            i = 0;
    }

    /**
     * We've changed messages, so reset the current position.
     */
    global->set_message_offset(0);
    delete( find );
    return 0;
}


/**
 * Scroll the index list up.
 */
int scroll_index_up(lua_State * L)
{
    int step = lua_tonumber(L, -1);

    CGlobal *global = CGlobal::Instance();
    int cur = global->get_selected_message();
    cur -= step;

    global->set_selected_message(cur);

    /**
     * We've changed messages, so reset the current position.
     */
    global->set_message_offset(0);

    return (0);
}

