/*
 * regexp_lua.cc - Export the `Regexp` object to Lua.
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

#include <pcrecpp.h>
#include <vector>
#include <iostream>


/**
 * @file regexp_lua.cc
 *
 * This file implements the trivial exporting of PCRE-based regular
 * expression matching to Lua.
 *
 * Lua-usage looks something like this:
 *
 *<code>
 * local res = Regexp:match( "[kh]emp$", "Steve Kemp" )<br/>
 * if ( res == true) then<br/>
 *   print "OK"<br/>
 * end<br/>
 *</code>
 *
 */

/**
 * Implementation of Regexp:match().
 *
 * This allows a pattern to be tested against a string.
 *
 * If the regexp contains no captures then `true` will be returned on
 * a successful match, otherwise `false`.
 *
 * If the regexp contains captures (up to ten) then they will be returned
 * as a table.
 */
int l_CRegexp_match(lua_State * l)
{
    const char *pattern = lua_tostring(l, 2);
    const char *input   = lua_tostring(l, 3);

    pcrecpp::RE_Options opt;
    opt.set_caseless(true);
    pcrecpp::RE re(pattern, opt);

    int n = re.NumberOfCapturingGroups();

    if (n == 0)
    {
        if (re.PartialMatch(input))
            lua_pushboolean(l , 1);
        else
            lua_pushboolean(l , 0);

        return 1;
    }

    std::string matches[10];
    const pcrecpp::Arg *args[10];

    int z = 0;
    pcrecpp::Arg arg0 = &matches[z];
    args[z++] = &arg0;

    pcrecpp::Arg arg1 = &matches[z];
    args[z++] = &arg1;

    pcrecpp::Arg arg2 = &matches[z];
    args[z++] = &arg2;

    pcrecpp::Arg arg3 = &matches[z];
    args[z++] = &arg3;

    pcrecpp::Arg arg4 = &matches[z];
    args[z++] = &arg4;

    pcrecpp::Arg arg5 = &matches[z];
    args[z++] = &arg5;

    pcrecpp::Arg arg6 = &matches[z];
    args[z++] = &arg6;

    pcrecpp::Arg arg7 = &matches[z];
    args[z++] = &arg7;

    pcrecpp::Arg arg8 = &matches[z];
    args[z++] = &arg8;

    pcrecpp::Arg arg9 = &matches[z];
    args[z++] = &arg9;

    pcrecpp::StringPiece in(input);

    int consumed;

    std::vector<std::string> r;

    do
    {
        if (re.DoMatch(in, re.UNANCHORED, &consumed, args, n))
        {
            in.remove_prefix(consumed);

            for (int t = 0; t < n && t < 10; t++)
            {
                r.push_back(matches[t]);
            }
        }
        else
            break;
    }
    while (true);

    lua_newtable(l);

    int i = 1;

    for (auto it = r.begin(); it != r.end(); ++it)
    {
        std::string value = (*it);

        lua_pushinteger(l, i);
        lua_pushstring(l, value.c_str());

        lua_settable(l, -3);

        i += 1;
    }

    return 1;
}

/**
 * Export the `Regexp` class to Lua.
 *
 * Bind the appropriate methods to that object.
 */
void InitRegexp(lua_State * l)
{
    luaL_Reg sFooRegs[] =
    {
        {"match", l_CRegexp_match},
        {NULL,       NULL}
    };
    luaL_newmetatable(l, "luaL_CRegexp");

#if LUA_VERSION_NUM == 501
    luaL_register(l, NULL, sFooRegs);
#elif LUA_VERSION_NUM == 502 || LUA_VERSION_NUM == 503
    luaL_setfuncs(l, sFooRegs, 0);
#else
#error We are only tested under Lua 5.1, 5.2, or 5.3.
#endif

    lua_pushvalue(l, -1);
    lua_setfield(l, -1, "__index");
    lua_setglobal(l, "Regexp");
}
