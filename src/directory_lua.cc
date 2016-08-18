/*
 * directory_lua.cc - Export the `Directory` object to Lua.
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


#include "directory.h"
#include "file.h"
#include "lua.h"

/**
 * @file directory_lua.cc
 *
 * This file implements the trivial exporting of our CDirectory class,
 * implemented in C++, to Lua.  Lua-usage looks something like this:
 *
 *<code>
 *   -- Does the given directory exist? <br/>
 *   if ( Directory:exists( "/etc" ) ) then <br/>
 *   &nbsp;&nbsp;print "All is well"<br/>
 *   end
 *</code>
 *
 */


/**
 * Implementation of `Directory:exists`.
 */
int l_CDirectory_exists(lua_State * l)
{
    CLuaLog("l_CDirectory_exists");

    const char *str = lua_tostring(l, 2);

    if (str == NULL)
    {
        lua_pushnil(l);
        return 1;
    }

    if (CDirectory::exists(str))
        lua_pushboolean(l , 1);
    else
        lua_pushboolean(l , 0);

    return 1;
}


/**
 * Implementation of `Directory:entries`.
 */
int l_CDirectory_entries(lua_State * l)
{
    CLuaLog("l_CDirectory_entries");

    const char *str = lua_tostring(l, 2);

    lua_newtable(l);

    if (str != NULL)
    {
        int i = 1;
        std::vector<std::string> entries = CDirectory::entries(str);

        for (auto it = entries.begin(); it != entries.end(); ++it)
        {
            std::string value = (*it);

            lua_pushinteger(l, i);
            lua_pushstring(l, value.c_str());

            lua_settable(l, -3);

            i += 1;
        }
    }

    return 1;
}


/**
 * Make a directory, creating all required parents.
 */
int l_CDirectory_mkdir(lua_State * l)
{
    CLuaLog("l_CDirectory_mkdir");

    const char *str = lua_tostring(l, 2);

    if (str != NULL)
    {
        CDirectory::mkdir_p(str);
    }

    return 0;
}


/**
 * Implementation of `Directory:is_maildir`.
 */
int l_CDirectory_is_maildir(lua_State * l)
{
    CLuaLog("l_CDirectory_is_maildir");

    const char *str = lua_tostring(l, 2);

    if (str == NULL)
    {
        lua_pushnil(l);
        return 1;
    }

    if (CFile::is_maildir(str))
        lua_pushboolean(l , 1);
    else
        lua_pushboolean(l , 0);

    return 1;
}

/**
 * Register the global `Directory` object to the Lua environment,
 * and setup our public methods upon which the user may operate.
 */
void InitDirectory(lua_State * l)
{
    luaL_Reg sFooRegs[] =
    {
        {"exists",  l_CDirectory_exists},
        {"entries", l_CDirectory_entries},
        {"mkdir",  l_CDirectory_mkdir},
        {"is_maildir", l_CDirectory_is_maildir},
        {NULL,      NULL}
    };
    luaL_newmetatable(l, "luaL_CDirectory");

#if LUA_VERSION_NUM == 501
    luaL_register(l, NULL, sFooRegs);
#elif LUA_VERSION_NUM == 502 || LUA_VERSION_NUM == 503
    luaL_setfuncs(l, sFooRegs, 0);
#else
#error We are only tested under Lua 5.1, 5.2, or 5.3.
#endif

    lua_pushvalue(l, -1);
    lua_setfield(l, -1, "__index");
    lua_setglobal(l, "Directory");
}
