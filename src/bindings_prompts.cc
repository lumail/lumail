/**
 * bindings_prompts.cc - Bindings for Lua prompting primitives.
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
#include <cursesw.h>
#include <cstdlib>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <string>
#include <unistd.h>



#include "bindings.h"
#include "debug.h"
#include "global.h"
#include "input.h"

#include "maildir.h"
#include "screen.h"
#include "utfstring.h"


/**
 * Prompt the user to select one of a small set of strings.
 */
int choose_string(lua_State * L)
{
    std::vector<std::string> choices;

    /**
     * Get all the options.
     */
    int n = lua_gettop(L);

    for (int i = 1; i <= n; i++)
    {
        const char *str = NULL;
        if (lua_isstring(L, i))
            str = lua_tostring(L, i);

        if ( str != NULL )
            choices.push_back( str );
    }

    /**
     * If there is only a single choice .. return it.
     */
    if ( choices.size() == 1 )
    {
        lua_pushstring(L, choices.at(0).c_str() );
        return 1;
    }

    assert(choices.size()>1);

    noecho();
    curs_set(0);

    /**
     * Prompt for clarification in the multiple-matches.
     */
    std::string choice = CScreen::choose_string( choices );

    /**
     * Reset the cursor.
     */
    curs_set(1);
    echo();

    lua_pushstring(L, choice.c_str() );
    return 1;
}

/**
 * Prompt for input.
 */
int prompt(lua_State * L)
{

    const char *str = NULL;

    /**
     * Get the prompt string.
     */
    if (lua_isstring(L, -1))
        str = lua_tostring(L, -1);

    if (str == NULL)
        return luaL_error(L, "Missing argument to prompt(..)");


    curs_set(1);
    echo();

    CScreen::clear_status();
    move(CScreen::height() - 1, 0);
    printw(str);

    timeout(-1000);

    std::string input = CScreen::get_line();
    noecho();
    timeout(1000);

    curs_set(0);

    CScreen::clear_status();
    lua_pushstring(L, input.c_str() );
    return 1;
}


/**
 * Prompt for "y/n".
 */
int prompt_yn(lua_State * L)
{
    const char *def_prompt = "(y)es, (n)o?";

    /**
     * Get the prompt string.
     */
    UTFString str;

    if (lua_isstring(L, -1))
        str = lua_tostring(L, -1);

    if (str.empty())
        str = def_prompt;

    /**
     * Push the characters and the prompt.
     */
    lua_pushstring(L, str.c_str());
    lua_pushstring(L,"ynYN");

    /**
     * Now we need to remap the return value:
     *
     *   y/Y -> true
     *   n/N -> false
     */
    if ( prompt_chars( L ) == 1 )
    {
        const char *response = lua_tostring(L,-1);
        if ( response != NULL &&
             ( response[0] == 'y' || response[0] == 'Y' ) )
        {
            lua_pushinteger(L, 1 );
            return 1;
        }
        if ( response != NULL &&
             ( response[0] == 'n' || response[0] == 'N' ) )
        {
            lua_pushinteger(L, 0 );
            return 1;
        }
    }
    return( 0 );
}


/**
 * Prompt for a single character from the set.
 */
int prompt_chars(lua_State *L)
{
    /**
     * Get the prompt string, and response set.
     */
    UTFString str;
    if (lua_isstring(L, -2))
        str = lua_tostring(L, -2);

    UTFString chars;
    if (lua_isstring(L, -1))
        chars = lua_tostring(L, -1);

    /**
     * Ensure both were set.
     */
    if (str.empty())
        return luaL_error(L, "Missing prompt to function prompt_chars(..)");
    if (chars.empty())
        return luaL_error(L, "Missing characters to function prompt_chars(..)");


    echo();

    int height = CScreen::height();
    timeout(-1000);

    bool done = true;

    while (done)
    {
        /**
         * Refresh the prompt.
         */
        CScreen::clear_status();
        move(height - 1, 0);
        printw(str.c_str());

        gunichar key;
        CInput::Instance()->get_wchar(&key);

        /**
         * See if the character was in the input string.
         */
        for( unsigned int i = 0; i < chars.size(); i++ )
        {
            if ( chars[i] == key )
            {

                Glib::ustring buf(1, key);
                lua_pushstring(L, buf.c_str());
                done = false;
            }
        }
    }
    noecho();
    curs_set(0);
    timeout(1000);

    CScreen::clear_status();
    return 1;
}


/**
 * Prompt for a maildir.
 */
int prompt_maildir(lua_State * L)
{
    CGlobal *global = CGlobal::Instance();
    int selected = 0;
    int height = CScreen::height();


    while (true)
    {
        CScreen::clear_main();

        std::vector<CMaildir*> folders = global->get_folders();

        int count = folders.size();
        if ( count < 1 )
        {
            lua_pushnil(L);
            return 1;
        }

        /**
         * NOTE: Selected starts at zero, hence the >=.
         */
        if ( selected >= count )
            selected = count;
        if ( selected < 0 )
            selected = 0;


        /**
         * Current selection
         */
        CMaildir *current = folders.at(selected);

        move(0,0);
        printw("Select a folder:");

        for (int row = 0; row < (height - 3); row++)
        {
            CMaildir *cur = NULL;
            if ((row + selected) < count)
            {
                cur = folders[row + selected];

                move( row+2, 0 );
                printw( "%s", cur->path().c_str() );
            }
        }

        gunichar key;
        if (CInput::Instance()->get_wchar(&key) == ERR)
        {
            /**
             * NOP
             */
        }
        if ( key == 'j' )
            selected += 1;
        if ( key == 'k' )
            selected -= 1;
        if ( key == '\n' )
        {
            lua_pushstring(L, current->path().c_str() );
            return 1;
        }
    }

    return 0;
}

