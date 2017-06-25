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


#include <sys/stat.h>

#include "file.h"
#include "lua.h"
#include "util.h"


/**
 * @file file_lua.cc
 *
 * This file implements the trivial exporting of our File class,
 * implemented in C++, to Lua.  Lua-usage looks something like this:
 *
 *<code>
 *   -- Does the given file exist? <br/>
 *   if ( File:exists( "/etc/foo" ) ) then<br />
 *   &nbsp;&nbsp;Panel:append( "Exists!") <br/>
 *   end<br />
 *</code>
 *
 */


/**
 * Get the basename of the given file.
 *
 * For example the input `/foo/bar/baz` will return the result `baz`.
 */
int l_CFile_basename(lua_State * l)
{
    CLuaLog("l_CFile_basename");

    const char *str = lua_tostring(l, 2);

    if (str == NULL)
    {
        lua_pushnil(l);
        return 1;
    }

    std::string result = CFile::basename(str);

    lua_pushstring(l , result.c_str());
    return 1;
}


/**
 * Copy the given file to the specified destination.
 */
int l_CFile_copy(lua_State * l)
{
    CLuaLog("l_CFile_copy");

    const char *src = lua_tostring(l, 2);
    const char *dst = lua_tostring(l, 3);

    CFile::copy(src, dst);

    return 0;
}


/**
 * Does the given file exist?
 */
int l_CFile_exists(lua_State * l)
{
    CLuaLog("l_CFile_exists");

    const char *str = lua_tostring(l, 2);

    if (str == NULL)
    {
        lua_pushnil(l);
        return 1;
    }

    if (CFile::exists(str))
        lua_pushboolean(l , 1);
    else
        lua_pushboolean(l , 0);

    return 1;
}


/**
 * Expand a file-path, in the same way a Unix shell would do.
 */
int l_CFile_expand(lua_State *l)
{
    CLuaLog("l_CFile_exists");

    const char *str = lua_tostring(l, 2);

    if (str == NULL)
    {
        lua_pushnil(l);
        return 1;
    }


    std::string result = shell_expand_path(str);
    lua_pushstring(l, result.c_str());

    return 1;
}


/**
 * Get details about the given file.
 *
 * This returns a Lua-table with values such as `size`, `uid`, `gid`, etc.
 */
int l_CFile_stat(lua_State * l)
{
    CLuaLog("l_CFile_stat");

    const char *str = lua_tostring(l, 2);
    struct stat sb;

    if (str == NULL)
    {
        lua_pushnil(l);
        return 1;
    }

    /*
     * If we fail to stat the entry return nil.
     */
    if ((stat(str, &sb) != 0))
    {
        lua_pushnil(l);
        return 1;
    }


    /*
     * Convert the mode of the file to a string.
     */
    char mode[8];
    snprintf(mode, 8, "0%o", sb.st_mode);


    /*
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

    lua_pushstring(l, "inode");
    lua_pushinteger(l, sb.st_ino);
    lua_settable(l, -3);

    lua_pushstring(l, "mtime");
    lua_pushinteger(l, sb.st_mtime);
    lua_settable(l, -3);

    lua_pushstring(l, "ctime");
    lua_pushinteger(l, sb.st_ctime);
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
 * Register the global `File` object to the Lua environment,
 * and setup our public methods upon which the user may operate.
 */
void InitFile(lua_State * l)
{
    luaL_Reg sFooRegs[] =
    {
        {"basename", l_CFile_basename},
        {"copy",     l_CFile_copy},
        {"exists",   l_CFile_exists},
        {"expand",   l_CFile_expand},
        {"stat",     l_CFile_stat},
        {NULL,       NULL}
    };
    luaL_newmetatable(l, "luaL_CFile");

#if LUA_VERSION_NUM == 501
    luaL_register(l, NULL, sFooRegs);
#elif LUA_VERSION_NUM == 502 || LUA_VERSION_NUM == 503
    luaL_setfuncs(l, sFooRegs, 0);
#else
#error We are only tested under Lua 5.1, 5.2, or 5.3.
#endif

    lua_pushvalue(l, -1);
    lua_setfield(l, -1, "__index");
    lua_setglobal(l, "File");
}
