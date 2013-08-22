/**
 * bindings.cc - Bindings for all functions callable from Lua.
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
 * Call a hook, with the given path.
 *
 * For example "on_edit_message", "on_send_message", or "on_message_aborted".
 */
void call_message_hook( const char *hook, const char *filename )
{
    CLua *lua = CLua::Instance();
    std::string cmd = std::string( hook ) + "(\"" + std::string(filename) + "\");";

    DEBUG_LOG( cmd );

    lua->execute( cmd );
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
 * Redraw the display.
 */
int refresh_display(lua_State * L)
{
    CScreen::clear_main();
    refresh();
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
 * Write a message to the status-bar.
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
 * scroll up/down the maildir list.
 */
int scroll_maildir_down(lua_State * L)
{
    int step = lua_tonumber(L, -1);

    CGlobal *global = CGlobal::Instance();

    int cur = global->get_selected_folder();
    cur += step;

    global->set_selected_folder(cur);

    return 0;
}


/**
 * Scroll the maildir list up.
 */
int scroll_maildir_up(lua_State * L)
{
    int step = lua_tonumber(L, -1);

    CGlobal *global = CGlobal::Instance();
    int cur = global->get_selected_folder();
    cur -= step;

    global->set_selected_folder(cur);

    return (0);
}


/**
 * Get the current offset into the maildir list.
 */
int maildir_offset(lua_State *L)
{
    CGlobal *global = CGlobal::Instance();
    int offset = global->get_selected_folder();

    lua_pushinteger(L, offset);
    return (1);
}


/**
 * Jump to the given entry in the maildir list.
 */
int jump_maildir_to(lua_State * L)
{
    int offset = lua_tonumber(L, -1);
    CGlobal *global = CGlobal::Instance();
    global->set_selected_folder(offset);

    return 0;
}





/**
 * scroll to the folder matching the pattern.
 */
int scroll_maildir_to(lua_State * L)
{
    const char *str = lua_tostring(L, -1);

    if (str == NULL)
        return luaL_error(L, "Missing argument to scroll_maildir_to(..)");


    /**
     * Lower-case version of the string.
     */
    std::string *find = new std::string(str);
    std::transform(find->begin(), find->end(), find->begin(), tolower);

    /**
     * get the current folders.
     */
    CGlobal *global = CGlobal::Instance();
    std::vector <CMaildir* > display = global->get_folders();
    int max = display.size();
    int selected = global->get_selected_folder();

    int i = selected + 1;

    while (i != selected)
    {
        if (i >= max)
            break;

        /**
         * Get the display-name of the folder.
         */
        CMaildir *cur = display[i];
        std::string p = cur->path();

        /**
         * Lower-case it.
         */
        std::transform(p.begin(), p.end(), p.begin(), tolower);

        if ( cur->matches_filter( find ) )
        {
            global->set_selected_folder(i);
            break;
        }
        i += 1;

        if (i >= max)
            i = 0;
    }

    delete ( find );
    return 0;
}


/**
 * Get the currently highlighted maildir folder.
 */
int current_maildir(lua_State * L)
{
    /**
     * get the current folders.
     */
    CGlobal *global = CGlobal::Instance();
    std::vector<CMaildir *> display = global->get_folders();
    int selected = global->get_selected_folder();

    CMaildir *x = display[selected];
    lua_pushstring(L, x->path().c_str());
    return 1;
}


/**
 * Select a maildir by the path.
 */
int select_maildir(lua_State *L)
{
    const char *path = lua_tostring(L, -1);
    if (path == NULL)
        return luaL_error(L, "Missing argument to select_maildir(..)");


    /**
     * get the current folders.
     */
    CGlobal *global = CGlobal::Instance();
    std::vector<CMaildir *> display = global->get_folders();
    int count = display.size();

    /**
     * Iterate over each one, and if we have a match then
     * set the selected one, and return true.
     */
    int i = 0;

    for( i = 0; i < count; i++ )
    {
        CMaildir *cur = display[i];

        if ( strcmp( cur->path().c_str(), path ) == 0 )
        {
            /**
             * The current folder has the correct
             * path, so we'll select it.
             */
            global->set_selected_folder( i );

            /**
             * Return: true
             */
            lua_pushboolean(L,1);
            return 1;

        }
    }

    /**
     * Return: false
     */
    lua_pushboolean(L,0);
    return 1;
}


/**
 * Return maildirs matching a given pattern.
 */
int maildirs_matching(lua_State *L)
{
    const char *pattern = lua_tostring(L, -1);
    if (pattern == NULL)
        return luaL_error(L, "Missing argument to maildirs_matching(..)");

    /**
     * Get all maildirs.
     */
    CGlobal *global = CGlobal::Instance();
    std::vector<CMaildir *> folders = global->get_folders();
    std::vector<CMaildir *>::iterator it;

    /**
     * create a new table.
     */
    lua_newtable(L);

    /**
     * Lua indexes start at one.
     */
    int i = 1;

    /**
     * We need to use a pointer to string for our
     * testing-function.
     */
    std::string *filter  = new std::string( pattern );

    /**
     * For each maildir - add it to the table if it matches.
     */
    for (it = folders.begin(); it != folders.end(); ++it)
    {
        CMaildir *f = (*it);


        if ( f->matches_filter( filter ) )
        {
            lua_pushnumber(L,i);
            lua_pushstring(L,(*it)->path().c_str());
            lua_settable(L,-3);
            i++;
        }
    }

    /**
     * Avoid leaking the filter.
     */
    delete( filter );

    return 1;
}


/**
 * Count the visible maildir folders.
 */
int count_maildirs(lua_State *L)
{
    CGlobal *global = CGlobal::Instance();

    std::vector<CMaildir *> folders = global->get_folders();
    lua_pushinteger(L, folders.size() );
    return 1;
}

/**
 * Get the names of all currently visible maildirs
 */
int current_maildirs(lua_State *L)
{
    CGlobal *global = CGlobal::Instance();
    std::vector<CMaildir *> display = global->get_folders();
    std::vector<CMaildir *>::iterator it;

    /**
     * Create the table.
     */
    lua_newtable(L);

    int i = 1;
    for (it = display.begin(); it != display.end(); ++it)
    {
        lua_pushnumber(L,i);
        lua_pushstring(L,(*it)->path().c_str());
        lua_settable(L,-3);
        i++;
    }

    return 1;
}

/**
 * Get the currently highlighted message-path.
 */
int current_message(lua_State * L)
{
    /**
     * Get the currently selected message.
     */
    CMessage *msg = get_message_for_operation( NULL );
    if ( msg == NULL )
    {
        CLua *lua = CLua::Instance();
        lua->execute( "msg(\"" MISSING_MESSAGE "\");" );
        return( 0 );
    }

    /**
     * If that succeeded store the path.
     */
    std::string source = msg->path();
    if ( !source.empty() )
    {
        lua_pushstring(L, source.c_str());
        return(1);
    }
    else
    {
        return 0;
    }
}



/**
 * Count the lines in the current message.
 */
int count_lines(lua_State * L)
{
    /**
     * Get the currently selected message.
     */
    CMessage *msg = get_message_for_operation( NULL );
    if ( msg == NULL )
    {
        CLua *lua = CLua::Instance();
        lua->execute( "msg(\"" MISSING_MESSAGE "\");" );
        return( 0 );
    }

    /**
     * If that succeeded get the body.
     */
    std::vector<UTFString> body = msg->body();
    lua_pushinteger(L, body.size() );
    return 1;
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
 * Count messages in the selected folder(s).
 */
int count_messages(lua_State * L)
{
    CGlobal *global = CGlobal::Instance();
    std::vector<CMessage *> *messages = global->get_messages();

    lua_pushinteger(L, messages->size() );
    return 1;
}



/**
 * Offset within the message we're displaying.
 */
int message_offset(lua_State * L)
{
    /**
     * How many lines we've scrolled down the message.
     */
    CGlobal *global = CGlobal::Instance();
    int offset = global->get_message_offset();

    lua_pushinteger(L, offset);
    return (1);
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

