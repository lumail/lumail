/**
 * bindings_text.cc - Bindings for text-display.
 *
 * This file is part of lumail: http://lumail.org/
 *
 * Copyright (c) 2013-2014 by Steve Kemp.  All rights reserved.
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
#include <cursesw.h>
#include <cstdlib>
#include <pcrecpp.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <string>
#include <unistd.h>



#include "bindings.h"
#include "debug.h"
#include "file.h"
#include "global.h"
#include "input.h"

#include "maildir.h"
#include "screen.h"
#include "utfstring.h"


/**
 * Jump to the given line of text.
 */
int jump_text_to(lua_State *L)
{
    int offset = lua_tonumber(L, -1);
    if ( offset < 0 )
        offset = 0;

    CGlobal *global = CGlobal::Instance();
    global->set_text_offset(offset);
    return (0);
}

int text_offset(lua_State *L)
{
    CGlobal *global = CGlobal::Instance();
    int offset = global->get_text_offset();

    assert(offset >= 0);
    lua_pushinteger(L, offset);

    return 1;
}

/**
 * Scroll the text down.
 */
int scroll_text_down(lua_State *L)
{
    int step = lua_tonumber(L, -1);

    CGlobal *global = CGlobal::Instance();
    int cur = global->get_text_offset();

    cur += step;

    global->set_text_offset(cur);
    return (0);
}


/**
 * Scroll the text to the given pattern.
 */
int scroll_text_to( lua_State *L)
{
    const char *str = NULL;

    if (lua_isstring(L, -1))
        str = lua_tostring(L, 1);

    if (str == NULL)
        return luaL_error(L, "Missing argument to scroll_text_to(..)");

    /**
     * Get the text, and the current offset.
     */
    CGlobal *global = CGlobal::Instance();
    std::vector<UTFString> text = global->get_text();
    size_t cur_offset           = global->get_text_offset();

    if ( text.size() < 1 )
        return 0;

    size_t offset = cur_offset;
    offset += 1;
    if ( offset >= text.size() )
        offset = 0;

    /**
     * Iterate over the text
     */
    while( offset != cur_offset )
    {
        UTFString line = "";
        line = text.at(offset);

        if (pcrecpp::RE(str, pcrecpp::RE_Options().set_caseless(true)).PartialMatch(line.c_str()) )
        {
            /**
             * We found a match.  Jump to it.
             */
            global->set_text_offset(offset);
            return 0;
        }

        /**
         * Next line.
         */
        offset += 1;
        if ( offset >= text.size() )
            offset = 0;
    }

    return 0;
}

/**
 * Scroll the text up.
 */
int scroll_text_up(lua_State *L)
{
    int step = lua_tonumber(L, -1);
    CGlobal *global = CGlobal::Instance();


    int cur = global->get_text_offset();
    cur -= step;

    if ( cur < 0 )
        cur = 0;

    global->set_text_offset(cur);
    return (0);
}


/**
 * View the contents of the given file.
 */
int show_file_contents(lua_State *L)
{
    const char *file = NULL;

    if (lua_isstring(L, -1))
        file = lua_tostring(L, 1);

    /**
     * Ensure we have a file.
     */
    if ( file == NULL )
        return luaL_error(L, "Missing argument for show_file_contents(..)");

    /**
     * Ensure the file exists.
     */
    if ( ! CFile::exists( file ) )
        return luaL_error(L, "File not found for show_file_contents(..)");


    FILE* f = fopen(file, "r");
    std::vector<UTFString> buf;

    char buffer[16384] = { '\0' };
    while(!feof(f))
    {
        if(fgets(buffer, sizeof(buffer)-1, f) != NULL)
            buf.push_back( buffer );
    }
    fclose(f);

    CGlobal *global = CGlobal::Instance();
    global->set_text( buf );
    global->set_text_offset( 0 );
    return 0;
}


/**
 * View the specified text.
 */
int show_text(lua_State *L)
{
    std::vector<UTFString> buf;

    /**
     * Ensure we have a table.
     */
    if (lua_type(L, -1)!=LUA_TTABLE)
    {
        lua_pop(L, 1);

        return 0;
    }

    lua_pushnil(L);

    while (lua_next(L, -2))
    {
        const char *d  = lua_tostring(L, -1);

        buf.push_back( d );
        lua_pop( L , 1);
    }

    /**
     * Cleanup the table.
     */
    lua_pop(L,1);

    CGlobal *global = CGlobal::Instance();
    global->set_text( buf );
    global->set_text_offset( 0 );

    return 0;
}
