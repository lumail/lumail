/**
 * bindings_mime.cc - Bindings for all MIME-related Lua primitives.
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
#include <cstdlib>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>


#include "bindings.h"
#include "debug.h"
#include "file.h"
#include "global.h"
#include "lang.h"
#include "lua.h"
#include "message.h"




/**
 * Get a table of attachment names for this mail.
 */
int attachments(lua_State *L)
{
    /**
     * Get the path (optional).
     */
    const char *str = lua_tostring(L, -1);

    CMessage *msg = get_message_for_operation( str );
    if ( msg == NULL )
    {
        CLua *lua = CLua::Instance();
        lua->execute( "msg(\"" MISSING_MESSAGE "\");" );
        return( 0 );
    }

    /**
     * Count the attachments.
     */
    std::vector<std::string> attachments = msg->attachments();

    /**
     * create a new table.
     */
    lua_newtable(L);

    /**
     * Lua indexes start at one.
     */
    int i = 1;

    /**
     * For each attachment, add it to the table.
     */
    for (std::string name : attachments)
    {
        lua_pushnumber(L,i);
        lua_pushstring(L,name.c_str());
        lua_settable(L,-3);
        i++;
    }


    if ( str != NULL )
        delete( msg );

    return( 1 );

}


/**
 * Count attachments in this mail.
 */
int count_attachments(lua_State *L)
{
    /**
     * Get the path (optional).
     */
    const char *str = lua_tostring(L, -1);

    CMessage *msg = get_message_for_operation( str );
    if ( msg == NULL )
    {
        CLua *lua = CLua::Instance();
        lua->execute( "msg(\"" MISSING_MESSAGE "\");" );
        return( 0 );
    }

    /**
     * Count the attachments.
     */
    std::vector<std::string> attachments = msg->attachments();
    int count = attachments.size();

    /**
     * Setup the return values.
     */
    lua_pushinteger(L, count );

    if ( str != NULL )
        delete( msg );

    return( 1 );

}


/**
 * Count the MIME-parts in this message.
 */
int count_body_parts(lua_State *L)
{
    /**
     * Get the path (optional).
     */
    const char *str = lua_tostring(L, -1);

    CMessage *msg = get_message_for_operation( str );
    if ( msg == NULL )
    {
        CLua *lua = CLua::Instance();
        lua->execute( "msg(\"" MISSING_MESSAGE "\");" );
        return( 0 );
    }

    /**
     * Get the parts, and store their count.
     */
    std::vector<std::string> parts = msg->body_mime_parts();
    int count = parts.size();

    /**
     * Setup the return values.
     */
    lua_pushinteger(L, count );

    if ( str != NULL )
        delete( msg );

    return( 1 );
}


/**
 * Return the single body-part from the message.
 */
int get_body_part(lua_State *L)
{
    /**
     * Get the path to save to.
     */
    int offset       = lua_tointeger(L,-1);

    CMessage *msg = get_message_for_operation(NULL);
    if ( msg == NULL )
    {
        CLua *lua = CLua::Instance();
        lua->execute( "msg(\"" MISSING_MESSAGE "\");" );
        return( 0 );
    }

    /**
     * Get the MIME-parts
     */
    std::vector<std::string> parts = msg->body_mime_parts();
    int count                      = parts.size();

    /**
     * Out of range: return false.
     */
    if ( ( offset < 1 ) || ( offset > count ) )
    {
        lua_pushboolean(L,0);
        return 1;
    }

    /**
     * Where we'll store the size/data.
     */
    char *result = NULL;
    size_t len = 0;

    if ( msg->get_body_part(offset, &result, &len ) )
    {
        lua_pushlstring(L, result, len);
        free(result);

        return 1;
    }
    else
    {
        lua_pushnil( L );
        return 1;
    }
}


/**
 * Return a table of the body-parts this message possesses.
 */
int get_body_parts(lua_State *L)
{
    /**
     * Get the path (optional).
     */
    const char *str = lua_tostring(L, -1);

    CMessage *msg = get_message_for_operation( str );
    if ( msg == NULL )
    {
        CLua *lua = CLua::Instance();
        lua->execute( "msg(\"" MISSING_MESSAGE "\");" );
        return( 0 );
    }

    /**
     * Get the parts, and prepare to iterate over them.
     */
    std::vector<std::string> parts = msg->body_mime_parts();

    /**
     * create a new table.
     */
    lua_newtable(L);

    /**
     * Lua indexes start at one.
     */
    int i = 1;

    /**
     * For each attachment, add it to the table.
     */
    for (std::string name : parts)
    {
        lua_pushnumber(L,i);
        lua_pushstring(L,name.c_str());
        lua_settable(L,-3);
        i++;
    }

    if ( str != NULL )
        delete( msg );

    return( 1 );
}



/**
 * Does the given message have a part of the given type?  (e.g. text/plain.)
 */
int has_body_part(lua_State *L)
{
    /**
     * Get the content-type.
     */
    const char *type = lua_tostring(L, -1);

    CMessage *msg = get_message_for_operation( NULL );
    if ( msg == NULL )
    {
        CLua *lua = CLua::Instance();
        lua->execute( "msg(\"" MISSING_MESSAGE "\");" );
        return( 0 );
    }

    /**
     * Get the parts, and prepare to iterate over them.
     */
    std::vector<std::string> parts = msg->body_mime_parts();

    /**
     * Did we find at least one, part that has the specified type?
     */
    for (std::string ct : parts)
    {
        if ( strcmp( ct.c_str(), type ) == 0 )
        {
            /**
             * Found a matching type.
             */
            lua_pushboolean(L,1);
            return 1;
        }
    }

    lua_pushboolean(L, 0 );
    return( 1 );
}


/**
 * Save the specified attachment.
 */
int save_attachment(lua_State *L)
{
    /**
     * Get the path to save to.
     */
    int offset       = lua_tointeger(L,-2);
    const char *path = lua_tostring(L, -1);

    CMessage *msg = get_message_for_operation(NULL);
    if ( msg == NULL )
    {
        CLua *lua = CLua::Instance();
        lua->execute( "msg(\"" MISSING_MESSAGE "\");" );
        return( 0 );
    }

    /**
     * Get the attachments.
     */
    std::vector<std::string> attachments = msg->attachments();
    int count                            = attachments.size();

    /**
     * Out of range: return false.
     */
    if ( ( offset < 1 ) || ( offset > count ) )
    {
        lua_pushboolean(L,0);
        return 1;
    }

    /**
     * Save the message.
     */
    bool ret = msg->save_attachment( offset, path );


    if ( ret )
    {
        assert(CFile::exists( path));
        lua_pushboolean(L, 1 );
    }
    else
    {
        lua_pushboolean(L, 0 );
    }
    return( 1 );

}

