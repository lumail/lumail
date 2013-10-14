/**
 * bindings_maildir.cc - Bindings for all maildir-related Lua primitives.
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
#include <fstream>
#include <iostream>
#include <string.h>


#include "bindings.h"
#include "debug.h"
#include "file.h"
#include "global.h"
#include "lang.h"
#include "lua.h"
#include "maildir.h"
#include "message.h"
#include "utfstring.h"
#include "variables.h"


/**
 * Count the visible maildir folders.
 */
int count_maildirs(lua_State *L)
{
    /**
     * Get all maildirs.
     */
    CGlobal *global = CGlobal::Instance();
    std::vector<CMaildir *> folders = global->get_folders();


    /**
     * Store the count.
     */
    lua_pushinteger(L, folders.size() );
    return 1;
}


/**
 * Get the currently highlighted maildir folder.
 */
int current_maildir(lua_State * L)
{
    /**
     * get the current folders.
     */
    CGlobal *global = CGlobal::Instance();
    std::vector<CMaildir *> display = global->get_folders();

    /**
     * Get the selected object.
     */
    int selected = global->get_selected_folder();
    CMaildir *x = display[selected];
    assert(x != NULL);

    /**
     * Store the path.
     */
    lua_pushstring(L, x->path().c_str());
    return 1;
}


/**
 * Get the names of all currently visible maildirs
 */
int current_maildirs(lua_State *L)
{
    CGlobal *global = CGlobal::Instance();
    std::vector<CMaildir *> display = global->get_folders();

    /**
     * Create the table.
     */
    lua_newtable(L);

    int i = 1;
    for (CMaildir * folder : display)
    {
        lua_pushnumber(L,i);
        lua_pushstring(L,folder->path().c_str());
        lua_settable(L,-3);
        i++;
    }

    return 1;
}


/**
 * Jump to the given entry in the maildir list.
 */
int jump_maildir_to(lua_State * L)
{
    int offset = lua_tonumber(L, -1);
    if ( offset < 0 )
        offset = 0;

    CGlobal *global = CGlobal::Instance();
    global->set_selected_folder(offset);

    return 0;
}


/**
 * Get the current offset into the maildir list.
 */
int maildir_offset(lua_State *L)
{
    CGlobal *global = CGlobal::Instance();
    int offset = global->get_selected_folder();
    assert(offset >=0);

    lua_pushinteger(L, offset);
    return (1);
}

/**
 * Return maildirs matching a given pattern.
 */
int maildirs_matching(lua_State *L)
{
    const char *pattern = lua_tostring(L, -1);
    if (pattern == NULL)
        return luaL_error(L, "Missing argument to maildirs_matching(..)");

    /**
     * Get all maildirs.
     */
    CGlobal *global = CGlobal::Instance();
    std::vector<CMaildir *> folders = global->get_folders();

    /**
     * create a new table.
     */
    lua_newtable(L);

    /**
     * Lua indexes start at one.
     */
    int i = 1;

    /**
     * We need to use a pointer to string for our
     * testing-function.
     */
    std::string *filter  = new std::string( pattern );

    /**
     * For each maildir - add it to the table if it matches.
     */
    for (CMaildir * folder : folders)
    {
        if ( folder->matches_filter( filter ) )
        {
            lua_pushnumber(L,i);
            lua_pushstring(L,folder->path().c_str());
            lua_settable(L,-3);
            i++;
        }
    }

    /**
     * Avoid leaking the filter.
     */
    delete( filter );

    return 1;
}


/**
 * scroll down the maildir list.
 */
int scroll_maildir_down(lua_State * L)
{
    int step = lua_tonumber(L, -1);

    CGlobal *global = CGlobal::Instance();

    int cur = global->get_selected_folder();
    cur += step;

    global->set_selected_folder(cur);

    return 0;
}


/**
 * scroll to the folder matching the pattern.
 */
int scroll_maildir_to(lua_State * L)
{
    const char *str = lua_tostring(L, -1);

    if (str == NULL)
        return luaL_error(L, "Missing argument to scroll_maildir_to(..)");


    /**
     * Lower-case version of the string.
     */
    std::string *find = new std::string(str);
    std::transform(find->begin(), find->end(), find->begin(), tolower);

    /**
     * get the current folders.
     */
    CGlobal *global = CGlobal::Instance();
    std::vector <CMaildir* > display = global->get_folders();
    int max = display.size();
    int selected = global->get_selected_folder();

    int i = selected + 1;

    while (i != selected)
    {
        if (i >= max)
            break;

        /**
         * Get the display-name of the folder.
         */
        CMaildir *cur = display[i];
        std::string p = cur->path();

        /**
         * Lower-case it.
         */
        std::transform(p.begin(), p.end(), p.begin(), tolower);

        if ( cur->matches_filter( find ) )
        {
            global->set_selected_folder(i);
            break;
        }
        i += 1;

        if (i >= max)
            i = 0;
    }

    delete ( find );
    return 0;
}

/**
 * Scroll the maildir list up.
 */
int scroll_maildir_up(lua_State * L)
{
    int step = lua_tonumber(L, -1);

    CGlobal *global = CGlobal::Instance();
    int cur = global->get_selected_folder();
    cur -= step;

    if ( cur < 0 )
        cur = 0;

    global->set_selected_folder(cur);

    return (0);
}


/**
 * Select a maildir by the path.
 */
int select_maildir(lua_State *L)
{
    const char *path = lua_tostring(L, -1);
    if (path == NULL)
        return luaL_error(L, "Missing argument to select_maildir(..)");


    /**
     * get the current folders.
     */
    CGlobal *global = CGlobal::Instance();
    std::vector<CMaildir *> display = global->get_folders();
    int count = display.size();

    /**
     * Iterate over each one, and if we have a match then
     * set the selected one, and return true.
     */
    int i = 0;

    for( i = 0; i < count; i++ )
    {
        CMaildir *cur = display[i];

        if ( strcmp( cur->path().c_str(), path ) == 0 )
        {
            /**
             * The current folder has the correct
             * path, so we'll select it.
             */
            global->set_selected_folder( i );

            /**
             * Return: true
             */
            lua_pushboolean(L,1);
            return 1;

        }
    }

    /**
     * Return: false
     */
    lua_pushboolean(L,0);
    return 1;
}

