/**
 * bindings_file.cc - Bindings for file-related Lua primitives.
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
#include <cstdlib>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <string>
#include <unistd.h>



#include "bindings.h"
#include "debug.h"
#include "file.h"
#include "global.h"
#include "maildir.h"



/**
 * Create a new maildir.
 */
int create_maildir(lua_State *L)
{
    const char *path = NULL;

    if (lua_isstring(L, -1))
        path = lua_tostring(L, -1);

    if (path == NULL)
        return luaL_error(L, "Missing argument to create_maildir(..)");

    if ( CMaildir::create( path ) )
        lua_pushboolean(L,1);
    else
        lua_pushboolean(L,0);

    CGlobal *global = CGlobal::Instance();
    global->update_maildirs();

    return 1;
}


/**
 * Change the current working directory.
 */
int cd(lua_State *L)
{
    const char *path = NULL;
    if (lua_isstring(L, -1))
        path = lua_tostring(L, -1);

    if (path == NULL)
        return luaL_error(L, "Missing argument to cd(..)");

    /**
     * We don't care if this succeeds or not.
     */
    int unused __attribute__((unused));
    unused = chdir(path);

    return 0;
}


/**
 * Return the current working directory.
 */
int cwd(lua_State *L)
{
    std::string path = getcwd(NULL,0);
    lua_pushstring(L, path.c_str() );
    return 1;
}


/**
 * Delete an existing maildir.
 */
int delete_maildir(lua_State *L)
{
    const char *path = NULL;

    if (lua_isstring(L, -1))
        path = lua_tostring(L, -1);

    if (path == NULL)
        return luaL_error(L, "Missing argument to delete_maildir(..)");

    /**
     * Ensure that the path specified is a maildir - return
     * false for failure if it is not.
     */
    if ( ! CMaildir::is_maildir( path ) )
    {
        lua_pushboolean(L,0);
        return 1;
    }

    /**
     * We're going to need to delete the directories beneath the
     * maildir.
     *
     * NOTE: The order is important here.
     */
    std::vector<std::string> dirs;
    dirs.push_back(std::string(path) + "/cur");
    dirs.push_back(std::string(path) + "/new");
    dirs.push_back(std::string(path) + "/tmp");
    dirs.push_back(path);

    /**
     * Delete each directory.
     */
    int ret = 1;

    for( std::string dir : dirs )
    {
        /**
         * If we've had no error .. continue to delete.
         */
        if ( ret == 1 )
        {
            if (rmdir( dir.c_str()) != 0 )
            {
                ret = 0;

#ifdef LUMAIL_DEBUG
    std::string dm = "delete_maildir(";
    dm += dir;
    dm += ") -> failed;";
    DEBUG_LOG( dm );
#endif
            }
        }
    }

    /**
     * Refresh our maildir list.
     */
    CGlobal *global = CGlobal::Instance();
    global->update_maildirs();


    lua_pushboolean(L,ret);
    return 1;
}

/**
 * Is the given path an executable?
 */
int executable(lua_State *L)
{
    const char *str = NULL;

    if (lua_isstring(L, -1))
        str = lua_tostring(L, -1);

    if (str == NULL)
        return luaL_error(L, "Missing argument to is_directory(..)");

    if ( CFile::executable( str ) )
        lua_pushboolean(L,1);
    else
        lua_pushboolean(L,0);

    return 1;
}

/**
 * Does the given file exist?
 */
int file_exists(lua_State *L)
{
    const char *str = NULL;

    if (lua_isstring(L, -1))
        str = lua_tostring(L, -1);

    if (str == NULL)
        return luaL_error(L, "Missing argument to file_exists(..)");

    if ( CFile::exists( str ) )
        lua_pushboolean(L,1);
    else
        lua_pushboolean(L,0);

    return 1;
}

/**
 * Is the given path a directory?
 */
int is_directory(lua_State *L)
{
    const char *str = NULL;

    if (lua_isstring(L, -1))
        str = lua_tostring(L, -1);

    if (str == NULL)
        return luaL_error(L, "Missing argument to is_directory(..)");

    if ( CFile::is_directory( str ) )
        lua_pushboolean(L,1);
    else
        lua_pushboolean(L,0);

    return 1;
}

/**
 * Is the given path a maildir?
 */
int is_maildir(lua_State *L)
{
    const char *str = lua_tostring(L, -1);
    if (str == NULL)
        return luaL_error(L, "Missing argument to is_maildir(..)");

    if ( CMaildir::is_maildir( str ) )
        lua_pushboolean(L,1);
    else
        lua_pushboolean(L,0);

    return 1;
}

/**
 * Load *.lua from the given directory.
 */
int load_directory(lua_State *L)
{

    const char *path = NULL;

    if (lua_isstring(L, -1))
        path = lua_tostring(L, -1);

    if (path == NULL)
        return luaL_error(L, "Missing argument to load_directory(..)");

    /**
     * The Lua helper.
     */
    CLua *lua = CLua::Instance();

    /**
     * Get the files in the directory.
     */
    std::vector<std::string> files = CFile::files_in_directory( path );
    int count = 0;

    for (std::string file : files)
    {
        /**
         * If the file ends in .lua then load it.
         */
        size_t offset = file.rfind('.');
        if(offset != std::string::npos)
        {
            /**
             * Get the lower-case version.
             */
            std::string extension = file.substr(offset+1);
            std::transform(extension.begin(), extension.end(), extension.begin(), tolower );

            if ( strcmp( "lua", extension.c_str() ) == 0 )
            {
                lua->load_file( file );
                count += 1;
            }

        }
    }

    /**
     * Return the number of files read.
     */
    lua_pushinteger(L, count );
    return 1;
}

