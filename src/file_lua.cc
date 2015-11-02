/*
 * file_lua.cc - Export the `File` object to Lua.
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


extern "C"
{
#include <lua.h>
#include <lauxlib.h>
#include <lualib.h>
}

#include <sys/stat.h>

#include "file.h"


/**
 * Does the given file exist?
 */
int l_CFile_exists(lua_State * l)
{
    const char *str = lua_tostring(l, 2);

    if (str != NULL)
    {
        if (CFile::exists(str))
            lua_pushboolean(l , 1);
        else
            lua_pushboolean(l , 0);

        return 1;
    }

    return 0;
}


/**
 * Get details about the given file.
 */
int l_CFile_stat(lua_State * l)
{
    const char *str = lua_tostring(l, 2);
    struct stat sb;

    if (str == NULL)
    {
        return 0;
    }

    /**
     * If we fail to stat the entry return nil.
     */
    if ((stat(str, &sb) != 0))
    {
        lua_pushnil(l);
        return 1;
    }


    /**
     * Convert the mode of the file to a string.
     */
    char mode[8];
    snprintf(mode, 8, "0%o", sb.st_mode);


    /**
     * Push a table onto the stack, and return it.
     */
    lua_newtable(l);

    lua_pushstring(l, "size");
    lua_pushinteger(l, sb.st_size);
    lua_settable(l, -3);

    lua_pushstring(l, "uid");
    lua_pushinteger(l, sb.st_uid);
    lua_settable(l, -3);

    lua_pushstring(l, "gid");
    lua_pushinteger(l, sb.st_gid);
    lua_settable(l, -3);

    lua_pushstring(l, "mode");
    lua_pushstring(l, mode + 3);
    lua_settable(l, -3);

    lua_pushstring(l, "type");

    if (S_ISREG(sb.st_mode))
        lua_pushstring(l, "file");
    else if (S_ISDIR(sb.st_mode))
        lua_pushstring(l, "directory");
    else if (S_ISBLK(sb.st_mode))
        lua_pushstring(l, "block");
    else if (S_ISCHR(sb.st_mode))
        lua_pushstring(l, "char");
    else if (S_ISFIFO(sb.st_mode))
        lua_pushstring(l, "fifo");
    else if (S_ISLNK(sb.st_mode))
        lua_pushstring(l, "link");
    else if (S_ISSOCK(sb.st_mode))
        lua_pushstring(l, "socket");
    else
        lua_pushstring(l, "unknown");

    lua_settable(l, -3);


    return 1;
}


/**
 * Export the `Class` class to Lua.
 *
 * Bind the appropriate methods to that object.
 */
void InitFile(lua_State * l)
{
    luaL_Reg sFooRegs[] =
    {
        {"exists",  l_CFile_exists},
        {"stat",    l_CFile_stat},
        {NULL,      NULL}
    };
    luaL_newmetatable(l, "luaL_CFile");

#if LUA_VERSION_NUM == 501
    luaL_register(l, NULL, sFooRegs);
#elif LUA_VERSION_NUM == 502
    luaL_setfuncs(l, sFooRegs, 0);
#else
#error unsupported Lua version
#endif

    lua_pushvalue(l, -1);
    lua_setfield(l, -1, "__index");
    lua_setglobal(l, "File");
}
