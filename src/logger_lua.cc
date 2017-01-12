/*
 * logfile_lua.cc - Logfile wrapper for lua.
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


#include "logger.h"
#include "lua.h"


/**
 * @file logfile_lua.cc
 *
 * This file implements the trivial exporting of our Log-class to Lua.
 *
 * There is only a single method implemented, and usage looks like this:
 *
 *<code>
 *   -- Setup a logfile <br/>
 *   Config:set( "global.logfile" , "/path/to/log" )<br/>
 * <br/>
 *   -- Add a message<br />
 *   Log:append( os.date() .. "Hello, world!" )<br />
 *</code>
 *
 */


/**
 * Append the specified line to our logfile.
 *
 * **NOTE**: If a logfile hasn't been set then this is a NOP.
 */
int l_CLog_log(lua_State * L)
{
    CLuaLog("l_CLog_log");

    const char *level = lua_tostring(L, 2);
    const char *msg   = lua_tostring(L, 3);

    if (msg == NULL)
        return 0;

    CLogger *log = CLogger::instance();
    log->log(level, msg);
    return 0;
}


/**
 * Get/Set the log-level.
 *
 * **NOTE**: If a level hasn't been set then logging is a NOP.
 */
int l_CLog_level(lua_State * L)
{
    CLuaLog("l_CLog_level");

    CLogger *log = CLogger::instance();

    const char *level = lua_tostring(L, 2);
    if (level != NULL)
        log->set_level( level );

    /*
     * Return.
     */
    lua_pushstring(L, log->get_level().c_str());
    return 1;
}


/**
 * Register the global `Log` object to the Lua environment,
 * and setup our public methods upon which the user may operate.
 */
void InitLogfile(lua_State * l)
{
    luaL_Reg sFooRegs[] =
    {
        {"log",  l_CLog_log},
        {"level",  l_CLog_level},
        {NULL,      NULL}
    };
    luaL_newmetatable(l, "luaL_CLog");

#if LUA_VERSION_NUM == 501
    luaL_register(l, NULL, sFooRegs);
#elif LUA_VERSION_NUM == 502 || LUA_VERSION_NUM == 503
    luaL_setfuncs(l, sFooRegs, 0);
#else
#error We are only tested under Lua 5.1, 5.2, or 5.3.
#endif

    lua_pushvalue(l, -1);
    lua_setfield(l, -1, "__index");
    lua_setglobal(l, "Log");
}
