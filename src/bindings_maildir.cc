/**
 * bindings_maildir.cc - Bindings for all maildir-related Lua primitives.
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
    CMaildirList folders = global->get_folders();


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
    CMaildirList display = global->get_folders();

    /**
     * Get the selected object.
     */
    int selected = global->get_selected_folder();
    std::shared_ptr<CMaildir> x = display[selected];
    assert(x != NULL);

    /**
     * Push it onto the Lua stack.
     */
    push_maildir(L, x);
    return 1;
}


/**
 * Get the names of all currently visible maildirs
 */
int current_maildirs(lua_State *L)
{
    CGlobal *global = CGlobal::Instance();
    CMaildirList display = global->get_folders();

    /**
     * Create the table.
     */
    lua_newtable(L);

    int i = 1;
    for (std::shared_ptr<CMaildir> folder : display)
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
    const char *pattern = NULL;

    if (lua_isstring(L, -1))
        pattern = lua_tostring(L, -1);

    if (pattern == NULL)
        return luaL_error(L, "Missing argument to maildirs_matching(..)");

    /**
     * Get all maildirs.
     */
    CGlobal *global = CGlobal::Instance();
    CMaildirList folders = global->get_folders();

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
    for (std::shared_ptr<CMaildir> folder : folders)
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
    const char *str = NULL;

    if (lua_isstring(L, -1))
        str = lua_tostring(L, -1);

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
    std::vector <std::shared_ptr<CMaildir> > display = global->get_folders();
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
        std::shared_ptr<CMaildir> cur = display[i];
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
    const char *path = NULL;

    if (lua_isstring(L, -1))
        path = lua_tostring(L, -1);

    if (path == NULL)
        return luaL_error(L, "Missing argument to select_maildir(..)");


    /**
     * get the current folders.
     */
    CGlobal *global = CGlobal::Instance();
    CMaildirList display = global->get_folders();
    int count = display.size();

    /**
     * Iterate over each one, and if we have a match then
     * set the selected one, and return true.
     */
    int i = 0;

    for( i = 0; i < count; i++ )
    {
        std::shared_ptr<CMaildir> cur = display[i];

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

/**
 * Verify that an item on the Lua stack is a wrapped CMaildir, and return
 * a (shared) pointer to it of so.
 *
 * Returns NULL otherwise.
 */
static std::shared_ptr<CMaildir> check_maildir(lua_State *L, int index)
{
    void *ud = luaL_checkudata(L, index, "maildir_mt");
    if (ud)
    {
        /* Return a copy of the pointer */
        return *(static_cast<std::shared_ptr<CMaildir> *>(ud));
    }
    else
    {
        /* Invalid, so return a null pointer */
        return NULL;
    }
}

/**
 * Delete the Maildir pointer
 */
static int maildir_mt_gc(lua_State *L)
{
    void *ud = luaL_checkudata(L, 1, "maildir_mt");
    if (ud)
    {
        std::shared_ptr<CMaildir> *ud_maildir = static_cast<std::shared_ptr<CMaildir> *>(ud);

        /* Call the destructor */
        ud_maildir->~shared_ptr<CMaildir>();
    }
    return 0;
}

/**
 * Function which takes a CMaildir (userdata) and a string,
 * and checks whether the maildir path matches the pattern.
 */
static int lmaildir_matches_regexp(lua_State *L)
{
    std::shared_ptr<CMaildir> maildir = check_maildir(L, 1);
    const char *cfilt = lua_tostring(L, 2);
    if (!cfilt)
    {
        return luaL_error(L, "Invalid or missing pattern to matches_filter.");
    }
    std::string filt(cfilt);

    bool result = maildir->matches_regexp(&filt);
    /* Tidy up the stack.  cfilt above will no longer be valid. */
    lua_pop(L, 2);

    /* And return the result */
    lua_pushboolean(L, result);
    return 1;
}

/**
 * Read maildir fields
 */
static int maildir_mt_index(lua_State *L)
{
    std::shared_ptr<CMaildir> maildir = check_maildir(L, 1);
    if (maildir)
    {
        const char *name = luaL_checkstring(L, 2);
        if (strcmp(name, "name") == 0)
        {
            /* Return the maildir's name */
            lua_pushstring(L, maildir->name().c_str());
            return 1;
        }
        else if (strcmp(name, "path") == 0)
        {
            /* Return the maildir's path */
            lua_pushstring(L, maildir->path().c_str());
            return 1;
        }
        else if (strcmp(name, "unread_messages") == 0)
        {
            lua_pushinteger(L, maildir->unread_messages());
            return 1;
        }
        else if (strcmp(name, "total_messages") == 0)
        {
            lua_pushinteger(L, maildir->total_messages());
            return 1;
        }
        else if (strcmp(name, "matches_regexp") == 0)
        {
            /* We return the function which implements the method, which
             * will usually be called immediately with the CMaildir as
             * the first argument and any others.
             */
            lua_pushcfunction(L, lmaildir_matches_regexp);
            return 1;
        }
    }
    return 0;
}

/**
 * The maildir metatable entries.
 */
static const luaL_Reg maildir_mt_fields[] =
{
    { "__index", maildir_mt_index },
    { "__gc",    maildir_mt_gc },
    { NULL, NULL },  /* Terminator */
};

/**
 * Push the maildir metatable onto the Lua stack, creating it if needed.
 */
static void push_maildir_mt(lua_State *L)
{
    int created = luaL_newmetatable(L, "maildir_mt");
    if (created)
    {
        /* A new table was created, set it up now. */
        luaL_register(L, NULL, maildir_mt_fields);
    }
}

/**
 * Push a maildir onto the Lua stack as a userdata.
 *
 * Returns true on success.
 */
bool push_maildir(lua_State *L, std::shared_ptr<CMaildir> maildir)
{
    void *ud = lua_newuserdata(L, sizeof(std::shared_ptr<CMaildir>));
    if (!ud)
        return false;

    /* Construct a blank shared_ptr.  To be safe, make sure it's a valid
     * object before setting the metatable. */
    std::shared_ptr<CMaildir> *ud_maildir = new (ud) std::shared_ptr<CMaildir>();
    if (!ud_maildir)
    {
        /* Discard the userdata */
        lua_pop(L, 1);
        return false;
    }

    /* FIXME: check errors */
    push_maildir_mt(L);

    lua_setmetatable(L, -2);

    /* And now store the maildir pointer into the userdata */
    *ud_maildir = maildir;

    return true;
}

/**
 * Push a vector of CMaildirs onto the Lua stack as a table.
 *
 * Returns true on success.
 */
bool push_maildir_list(lua_State *L,
                       const CMaildirList &maildirs)
{
    lua_createtable(L, maildirs.size(), 0);
    for (size_t i=0; i<maildirs.size(); ++i)
    {
        if (!push_maildir(L, maildirs[i]))
            return false;

        /* Add to the table. */
        lua_rawseti(L, -2, i+1);
    }
    return true;
}

/**
 * Verify that an item on the Lua stack is a table of CMaildir, and return
 * this table converted back to a std::vector if so.
 *
 * Returns an empty vector otherwise.
 */
CMaildirList check_maildir_list(lua_State *L, int index)
{
    CMaildirList result;
    size_t size = lua_objlen(L, index);
    for (size_t i=1; i<=size; ++i)
    {
        std::shared_ptr<CMaildir> md;
        lua_rawgeti(L, index, i);
        md = check_maildir(L, -1);
        /* Ignore anything that isn't a CMaildir */
        if (md)
            result.push_back(check_maildir(L, -1));
        lua_pop(L, 1);
    }
    return result;
}
