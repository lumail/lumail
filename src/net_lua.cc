/*
 * net_lua.cc - Network functions bound to lua.
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

#include <netdb.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <unistd.h>


/**
 * Get details about the given file.
 */
int l_CNet_hostname(lua_State * L)
{
    /**
     * If the environmental varaible HOSTNAME
     * is set, use that.
     */
    const char *env = getenv("HOSTNAME");

    if (env != NULL)
    {
        lua_pushstring(L, env);
    }
    else
    {
        /*
         * Get the short version.
         */
        char res[1024];
        res[sizeof(res) - 1] = '\0';
        gethostname(res, sizeof(res) - 1);

        /*
         * Attempt to get the full vrsion.
         */
        struct hostent *hstnm;
        hstnm = gethostbyname(res);

        if (hstnm)
        {
            /*
             * Success.
             */
            lua_pushstring(L, hstnm->h_name);
        }
        else
        {
            /*
             * Failure: Return the short-version.
             */
            lua_pushstring(L, res);
        }
    }

    return 1;
}


/**
 * Export the utility functions to Lua.
 */
void InitNet(lua_State * l)
{
    luaL_Reg sFooRegs[] =
    {
        {"hostname",  l_CNet_hostname},
        {NULL,      NULL}
    };
    luaL_newmetatable(l, "luaL_CHostname");

#if LUA_VERSION_NUM == 501
    luaL_register(l, NULL, sFooRegs);
#elif LUA_VERSION_NUM == 502
    luaL_setfuncs(l, sFooRegs, 0);
#else
#error unsupported Lua version
#endif

    lua_pushvalue(l, -1);
    lua_setfield(l, -1, "__index");
    lua_setglobal(l, "Net");
}
