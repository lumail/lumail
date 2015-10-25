/**
 * $FILENAME - $TITLE
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
#include <algorithm>
#include <cstdlib>
#include <dirent.h>
#include <fcntl.h>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string.h>
#include <string>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <unordered_map>
#include <vector>
#include <gmime/gmime.h>


#include "file.h"
#include "maildir.h"
#include "message.h"



/**
 * Binding for CMaildir
 */
int
l_CMaildir_constructor (lua_State * l)
{
    const char *name = luaL_checkstring (l, 1);

    CMaildir **udata = (CMaildir **) lua_newuserdata (l, sizeof (CMaildir *));
    *udata = new CMaildir (name);

    luaL_getmetatable (l, "luaL_CMaildir");

    lua_setmetatable (l, -2);

    return 1;
}



/**
 * Is the object a CMaildir object?
 */
CMaildir *
l_CheckCMaildir (lua_State * l, int n)
{
    return *(CMaildir **) luaL_checkudata (l, n, "luaL_CMaildir");
}



/**
 * Return the path of this maildir object.
 */
int
l_CMaildir_path (lua_State * l)
{
    CMaildir *foo = l_CheckCMaildir (l, 1);

    lua_pushstring (l, foo->path ().c_str ());
    return 1;
}



/**
 * Count the messages in the directory.
 */
int
l_CMaildir_total_messages (lua_State * l)
{
    CMaildir *foo = l_CheckCMaildir (l, 1);
    lua_pushinteger (l, foo->total_messages ());
    return 1;
}



/**
 * Count the unread messages in the maildir-directory.
 */
int
l_CMaildir_unread_messages (lua_State * l)
{
    CMaildir *foo = l_CheckCMaildir (l, 1);
    lua_pushinteger (l, foo->unread_messages ());
    return 1;
}



/**
 * Does the maildir exist?
 */
int
l_CMaildir_exists (lua_State * l)
{
    CMaildir *foo = l_CheckCMaildir (l, 1);


    int n = lua_gettop (l);
    if (n != 1)
    {
	fprintf (stderr, "GOT A STRING?!\n");
    }

    if (foo->is_maildir ())
	lua_pushboolean (l, 1);
    else
	lua_pushboolean (l, 0);
    return 1;
}



/**
 * Return a C++ CMessage object for each message.
 *
 * Suspect this is broken - but it seems to work.
 */
int
l_CMaildir_messages (lua_State * l)
{
    CMaildir *m = l_CheckCMaildir (l, 1);

    std::vector < std::string > tmp = m->messages ();

    lua_createtable (l, tmp.size (), 0);
    int i = 0;

    for (std::vector < std::string >::iterator it = tmp.begin ();
	 it != tmp.end (); ++it)
    {
	CMessage **udata =
	    (CMessage **) lua_newuserdata (l, sizeof (CMessage *));
	*udata = new CMessage (*it);
	luaL_getmetatable (l, "luaL_CMessage");
	lua_setmetatable (l, -2);
	lua_rawseti (l, -2, i + 1);

	i++;
    }

    return 1;
}



/**
 * Destructor.
 */
int
l_CMaildir_destructor (lua_State * l)
{
    CMaildir *foo = l_CheckCMaildir (l, 1);
    delete foo;

    return 0;
}


/**
 * Get all maildirs beneath a given prefix.
 */
int
get_maildirs (lua_State * L)
{
    const char *prefix = luaL_checkstring (L, 1);

    std::vector < std::string > tmp = CFile::get_all_maildirs (prefix);

    lua_createtable (L, tmp.size (), 0);
    int i = 0;

    for (std::vector < std::string >::iterator it = tmp.begin ();
	 it != tmp.end (); ++it)
    {
	CMaildir **udata =
	    (CMaildir **) lua_newuserdata (L, sizeof (CMaildir *));
	*udata = new CMaildir (*it);
	luaL_getmetatable (L, "luaL_CMaildir");
	lua_setmetatable (L, -2);
	lua_rawseti (L, -2, i + 1);

	i++;
    }

    return 1;

}

/**
 * Somebody set us up the mapping.
 */
void
InitMaildir (lua_State * l)
{
    luaL_Reg sFooRegs[] = {
	{"new", l_CMaildir_constructor},
	{"path", l_CMaildir_path},
	{"total_messages", l_CMaildir_total_messages},
	{"unread_messages", l_CMaildir_unread_messages},
	{"messages", l_CMaildir_messages},
	{"exists", l_CMaildir_exists},
	{"__gc", l_CMaildir_destructor},
	{NULL, NULL}
    };
    luaL_newmetatable (l, "luaL_CMaildir");

#if LUA_VERSION_NUM == 501
    luaL_register (l, NULL, sFooRegs);
#elif LUA_VERSION_NUM == 502
    luaL_setfuncs (l, sFooRegs, 0);
#else
#error unsupported Lua version
#endif

    lua_pushvalue (l, -1);
    lua_setfield (l, -1, "__index");
    lua_setglobal (l, "Maildir");


    /**
     * Now add in the static method.
     */
    lua_pushcfunction (l, get_maildirs);
    lua_setglobal (l, "get_maildirs");
}
