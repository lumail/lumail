/**
 * bindings.cc - Bindings for all basic Lua primitives.
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
#include <cursesw.h>
#include <fstream>
#include <iostream>
#include <map>
#include <pcrecpp.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <netdb.h>
#include <unistd.h>


#include "bindings.h"
#include "debug.h"
#include "file.h"
#include "global.h"
#include "input.h"
#include "lang.h"
#include "lua.h"
#include "maildir.h"
#include "message.h"
#include "screen.h"
#include "utfstring.h"
#include "variables.h"





/**
 * Get a message from the given path.
 *
 * If the path is NULL then we allocate the message from the heap as the
 * currently selected message.
 */
CMessage *get_message_for_operation( const char *path )
{
    CMessage *msg = NULL;

    /**
     * Given a path?  Use it.
     */
    if ( path != NULL )
    {
        msg = new CMessage( path );

        DEBUG_LOG( "get_message_for_operation:"  + std::string(msg->header( "Subject" ) ) );

        return(msg);
    }


    /**
     * OK we're working with the currently selected message.
     */

    /**
     * Get all messages from the currently selected messages.
     */
    CGlobal *global = CGlobal::Instance();
    std::vector<CMessage *> *messages = global->get_messages();

    /**
     * The number of items we've found, and the currently selected one.
     */
    int count    = messages->size();
    int selected = global->get_selected_message();

    /**
     * No messages?
     *
     * NOTE: "selected" starts from zero, hence the ">=".
     */
    if ( ( count < 1 ) || ( selected >=  count ) )
        return NULL;
    else
    {
        DEBUG_LOG( "get_message_for_operation:"  + std::string(messages->at(selected)->header( "Subject" ) ) );

        return(  messages->at(selected) );
    }

}






/**
 * Exit the program, abnormally.
 */
int abort(lua_State * L)
{

    /**
     * Close curses.
     */
    endwin();

    /**
     * Shutdown GMime.
     */
    g_mime_shutdown();

    const char *str = lua_tostring(L, -1);

    if (str != NULL)
        std::cerr << str << std::endl;

    exit(1);

    /**
     * Never reached.
     */
    return 0;
}


/**
 * Alert: Draw a message and await explicit confirmation.
 */
int alert(lua_State * L)
{
    const int DEFAULT_TIMEOUT = 30;

    const char *str = luaL_checkstring(L, 1);
    int time_out    = luaL_optinteger(L, 2, DEFAULT_TIMEOUT);

    if (time_out < 0)
        return luaL_error(L, "timeout must be a positive integer.");
    if (str == NULL)
        return luaL_error(L, "Missing argument to alert(..)");

    /**
     * Cleanup
     */
    lua_pop(L, 2);

    echo();
    timeout(0);

    /**
     * Starting time.
     */
    time_t now = time( NULL);


    int height = CScreen::height() - 4;
    int width = CScreen::width() - 4;
    WINDOW *childwin = newwin(height, width, 2, 2);
    box(childwin, 0, 0);
    mvwaddstr(childwin, 2, 4, str);

    bool done = false;

    while( ! done )
    {
        refresh();
        char progress[100] = {'\0'};

        int elapsed = time(NULL) - now;

        memset(progress, '\0', sizeof(progress));
        snprintf( progress, sizeof(progress)-1, "%d/%d", elapsed, time_out );
        mvwaddstr(childwin, height-4, 10, "Press RET to continue, timeout in " );
        waddstr(childwin,progress);
        wrefresh(childwin);

        gunichar key;
        CInput::Instance()->get_wchar(&key);
        if ( key == '\n' )
            done = true;

        /**
         * Timeout.
         */
        if ( elapsed > time_out )
            done = true;
    }
    delwin(childwin);
    noecho();
    curs_set(0);
    timeout(1000);

    CScreen::clear_status();
    return 0;
}


/**
 * Clear the screen; but not the prompt.
 */
int clear(lua_State * L)
{
    CScreen::clear_main();
    return 0;
}


/**
 * Execute a program, resetting curses first.
 */
int exec(lua_State * L)
{
    const char *str = lua_tostring(L, -1);
    if (str == NULL)
        return luaL_error(L, "Missing argument to exec(..)");

    CScreen::clear_status();

    /**
     * Save the current state of the TTY
     */
    refresh();
    def_prog_mode();
    endwin();

    /* Run the command */
    int result __attribute__((unused));
    result = system(str);


    /**
     * Reset + redraw
     */
    reset_prog_mode();
    refresh();
    return 0;
}


/**
 * Exit the program.
 */
int exit(lua_State * L)
{
    /**
     * Close curses.
     */
    endwin();

    /**
     * Shutdown GMime.
     */
    g_mime_shutdown();

    CLua *lua = CLua::Instance();
    lua->execute("on_exit()");

    exit(0);
    return 0;
}


/**
 * Return a table of all known variables and their current
 * settings.
 */
int get_variables(lua_State *L )
{
    CGlobal *global = CGlobal::Instance();
    std::unordered_map<std::string,std::string *>  vars = global->get_variables();
    std::unordered_map<std::string, std::string *>::iterator iter;

    /**
     * Create the table.
     */
    lua_newtable(L);

    int i = 1;
    for (iter = vars.begin(); iter != vars.end(); ++iter )
    {
        std::string name = iter->first;
        std::string *val = iter->second;

        lua_pushstring(L,name.c_str() );

        if ( val != NULL )
            lua_pushstring(L,val->c_str());
        else
            lua_pushstring(L, "NULL" );

        lua_settable(L,-3);
        i++;
    }

    return 1;
}


/**
 * Return the hostname to Lua.
 */
int hostname(lua_State *L)
{
    /**
     * Get the short version.
     */
    char res[1024];
    res[sizeof(res)-1] = '\0';
    gethostname(res, sizeof(res)-1);

    /**
     * Attempt to get the full vrsion.
     */
    struct hostent *hstnm;
    hstnm = gethostbyname (res);
    if (hstnm)
    {
        /**
         * Success.
         */
        lua_pushstring(L,hstnm->h_name);
    }
    else
    {
        /**
         * Failure: Return the short-version.
         */
        lua_pushstring(L,res );
    }

    return 1;
}


/**
 * Log a message to the debug-log.
 */
int log_message(lua_State *L)
{
#ifdef LUMAIL_DEBUG
    const char *str = lua_tostring(L, -1);
    if (str == NULL)
        return luaL_error(L, "Missing argument to log_message(..)");

    /**
     * Log the message, and force it to be written immediately
     * bypassing the buffering.
     */
    CDebug::Instance()->debug( str, true );

#endif
    return 0;
}


/**
 * Dump the Lua stack to our debug-log.
 */
int lua_dump_stack(lua_State *L)
{
#ifdef LUMAIL_DEBUG
    CLua *lua = CLua::Instance();
    lua->dump_stack();
#endif
    return 0;
}


/**
 * Get the MIME-type of a file, based upon suffix.
 *
 * We use /etc/mime.types to do this properly.
 */
int mime_type(lua_State *L)
{
    /**
     * Map of file-suffix to MIME-types.
     */
    static std::map< std::string, std::string> table;

    /**
     * Default MIME-type if we can't find the real one.
     */
    const char *default_type = "application/octet-stream";

    /**
     * Get the file to test.
     */
    const char *file = lua_tostring(L, -1);
    if (file == NULL)
        return luaL_error(L, "Missing argument to mime_type(..)");

    /**
     * Attempt to find the extension.
     */
    std::string filename(file);
    size_t offset = filename.rfind('.');

    /**
     * If we found one.
     */
    if(offset != std::string::npos)
    {
        /**
         * Get the lower-case version.
         */
        std::string extension = filename.substr(offset+1);
        std::transform(extension.begin(), extension.end(), extension.begin(), tolower);

        /**
         * If we've not already populated our map..
         */
        if ( table.size() < 1 )
        {
            DEBUG_LOG( "MIME-type table empty.  Populating" );

            /**
             * Attempt to parse /etc/mime.types.
             */
            if ( CFile::exists("/etc/mime.types" ) )
            {
                DEBUG_LOG( "/etc/mime.types is present.  Using it." );

                std::vector<std::string> data;
                pcrecpp::RE regex("(.+?)(?:[\\r\\n\\t]+|$)");

                std::ifstream file("/etc/mime.types", std::ios::in);
                if (file.is_open())
                {
                    std::string line;
                    std::string piece;

                    while (!file.eof())
                    {
                        getline(file, line);
                        pcrecpp::StringPiece input(line.c_str());
                        while (regex.Consume(&input, &piece))
                            data.push_back(piece);

                        for (size_t i = 1; i < data.size(); i++)
                        {
                            /**
                             * The results here will be something like
                             *    ext1 ext2 ext3 -> type
                             */
                            std::string suffixes = data[i];
                            std::string m_type  = data[0];

                            /**
                             * Split suffixes on whitespace.
                             */
                            std::istringstream helper(suffixes);
                            std::string tmp;
                            while (std::getline(helper, tmp, ' '))
                            {
                                table[tmp] = m_type;
                                DEBUG_LOG( "MIME[" + tmp + "] -> " + m_type );
                            }

                            table[data[i]] = data[0];
                        }
                        data.clear();
                    }
                    file.close();
                }
            }

            /**
             * The previous code should have opened /etc/mime.types
             * and parsed it.
             *
             * However that file might be missing, or the parsing might
             * have failed.
             *
             * If that is the case then the map will be empty, so we'll
             * add some simple entries and leave it at that.
             */
            if ( table.size() < 1 )
            {
                DEBUG_LOG( "MIME-table empty.  Using our fixed list." );

                table[ "conf" ] = "text/plain";
                table[ "gif" ]  = "image/gif";
                table[ "htm" ]  = "text/html";
                table[ "html" ] = "text/html";
                table[ "jpeg" ] = "image/jpg";
                table[ "jpg" ]  = "image/jpg";
                table[ "png" ]  = "image/png";
                table[ "text" ] = "text/plain";
                table[ "txt" ]  = "text/plain";
            }
        }

        /**
         * Look up the value, and use the default if we can't find a match.
         */
        std::string value = table[ extension ];

        /**
         * Log the value.
         */
        DEBUG_LOG( "MIME type of " + filename + " [suffix:" + extension + "] is "
                   + value );

        if ( value.empty() )
        {
            DEBUG_LOG( "Using default MIME value: " + std::string(default_type)  );

            value = default_type;
        }

        lua_pushstring(L, value.c_str());
        return(1);
    }
    else
    {
        /**
         * No extension found.
         */
        lua_pushstring(L, default_type );
        return(1);
    }
}


/**
 * Write a message to the status-area.
 */
int msg(lua_State * L)
{
    char buf[1023] = { '\0' };

    /**
     * Get the type of argument.
     */
    int t = lua_type(L, -1);
    switch (t)
    {
    case LUA_TSTRING:
        snprintf(buf, sizeof(buf)-1,"%s", lua_tostring(L, -1));
        break;

    case LUA_TBOOLEAN:
        snprintf(buf, sizeof(buf)-1,lua_toboolean(L, -1) ? "true" : "false");
        break;

    case LUA_TNIL:
        snprintf(buf, sizeof(buf)-1, "nil" );
        break;

    case LUA_TNUMBER:
        snprintf(buf, sizeof(buf)-1,"%g", lua_tonumber(L, -1));
        break;
    default:
        snprintf(buf, sizeof(buf)-1,"Cannot print argument of type %s", lua_typename(L, t));
        break;
    }

    if (strlen(buf) < 1 )
        return luaL_error(L, "Missing argument to msg(..)");

    /**
     * Are we evaluating?
     */
    CGlobal *global = CGlobal::Instance();
    std::string *f  = global->get_variable( "eval_exit" );
    if ( ( f != NULL ) && ( strcmp( f->c_str(), "true" ) == 0 ) )
    {
        def_prog_mode();
        endwin();

        std::cout << buf << std::endl;

        reset_prog_mode();
        refresh();

        return 0;
    }

    CScreen::clear_status();
    move(CScreen::height() - 1, 0);
    printw("%s", buf);
    return 0;
}


/**
 * Redraw the display.
 */
int refresh_display(lua_State * L)
{
    CScreen::clear_main();
    refresh();
    return 0;
}


/**
 * Get the screen width.
 */
int screen_width(lua_State * L)
{
    lua_pushinteger(L, CScreen::width() );
    return 1;
}


/**
 * Get the screen height.
 */
int screen_height(lua_State * L)
{
    lua_pushinteger(L, CScreen::height() );
    return 1;
}


/**
 * Show brief help
 */
int show_help(lua_State * L)
{
    const char *str = lua_tostring(L, -1);

    if (str == NULL)
        return luaL_error(L, "Missing argument to help(..)");

    for(int i = 0; i < primitive_count; i++ )
    {
        const char *name = primitive_list[i].name;
        const char *help = primitive_list[i].help;

        if ( strcasecmp( name, str ) == 0 )
        {
            lua_pushstring(L, help );
            return( msg(L) );
        }
    }

    return 0;
}


/**
 * Sleep.
 */
int sleep(lua_State *L )
{
    int delay = lua_tointeger(L, -1);
    if (delay < 0 )
        return luaL_error(L, "positive integer expected for sleep(..)");

    sleep( delay );
    return 0;
}


/**
 * Stuff input into the input-buffer.
 */
int stuff(lua_State * L)
{
    UTFString str = lua_tostring(L, -1);

    if (str.empty())
        return luaL_error(L, "Missing argument to stuff(..)");

    CInput::Instance()->add( UTFString( str ) );

    return 0;
}
