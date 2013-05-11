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

#include <stdio.h>
#include <algorithm>
#include <cstdlib>
#include <iostream>
#include <fstream>
#include <string.h>
#include <malloc.h>
#include <ncurses.h>
#include <unistd.h>

#include "maildir.h"
#include "lua.h"
#include "global.h"
#include "screen.h"
#include "lua.h"

/**
 * Set the maildir-prefix
 */
int maildir_prefix(lua_State * L)
{
    const char *str = lua_tostring(L, -1);
    CGlobal *g = CGlobal::Instance();

    if (str != NULL) {
      if ( !CMaildir::isDirectory( str ) )
	return luaL_error(L, "The specified prefix is not a Maildir" );

	g->set_maildir_prefix(new std::string(str));
    }

    std::string * s = g->get_maildir_prefix();
    lua_pushstring(L, s->c_str());
    return 1;
}

/**
 * Set the index-format
 */
int index_format(lua_State * L)
{
    const char *str = lua_tostring(L, -1);
    CGlobal *g = CGlobal::Instance();

    if (str != NULL)
	g->set_index_format(new std::string(str));

    std::string * s = g->get_index_format();
    lua_pushstring(L, s->c_str());
    return 1;
}

/**
 * Get/Set the global lumail mode.
 */
int global_mode(lua_State * L)
{
    CGlobal *g = CGlobal::Instance();

    /**
     * get the argument, and if we have one set it.
     */
    const char *str = lua_tostring(L, -1);
    if (str != NULL)
	g->set_mode(new std::string(str));

    /**
     * Return the current/updated value.
     */
    std::string * s = g->get_mode();
    lua_pushstring(L, s->c_str());
    return 1;
}

/**
 * Limit the maildir display.
 */
int maildir_limit(lua_State * L)
{
    CGlobal *g = CGlobal::Instance();

    /**
     * get the argument, and if we have one set it.
     */
    const char *str = lua_tostring(L, -1);
    if (str != NULL)
	g->set_maildir_limit(new std::string(str));

    /**
     * Return the current/updated value.
     */
    std::string * s = g->get_maildir_limit();
    lua_pushstring(L, s->c_str());
    return 1;
}


/**
 * limit the display of messages.
 */
int index_limit(lua_State * L)
{
    CGlobal *g = CGlobal::Instance();

    /**
     * get the argument, and if we have one set it.
     */
    const char *str = lua_tostring(L, -1);
    if (str != NULL)
	g->set_index_limit(new std::string(str));

    /**
     * Return the current/updated value.
     */
    std::string * s = g->get_index_limit();
    lua_pushstring(L, s->c_str());
    return 1;
}


/**
 * Clear the screen.
 */
int clear(lua_State * L)
{
  /**
   * Clear all the screen - but not the prompt.
   */
  int width = CScreen::width();
  int height = CScreen::height();

  std::string blank = "";
  while( (int)blank.length() < width )
    blank += " ";

  for(int i = 0; i < ( height - 1 ); i++ )
    mvprintw( i, 0, "%s", blank.c_str() );

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
    endwin();

    CLua *lua = CLua::Instance();
    lua->callFunction("on_exit");

    exit(0);
    return 0;
}

/**
 * Execute a program.
 */
int exec(lua_State * L)
{
    const char *str = lua_tostring(L, -1);
    if (str == NULL)
	return luaL_error(L, "Missing argument to exec(..)");

    CScreen::clearStatus();

    /**
     * Save the current state of the TTY
     */
    refresh();
    def_prog_mode();
    endwin();

    /* Run the command */
    system(str);

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
    const char *str = lua_tostring(L, -1);

    if (str == NULL)
	return luaL_error(L, "Missing argument to msg(..)");

    CScreen::clearStatus();
    move(CScreen::height() - 1, 0);
    printw("%s", str);
    return 0;
}

/**
 * Prompt for input.
 */
int prompt(lua_State * L)
{
  /**
   * Get the prompt string.
   */
    const char *str = lua_tostring(L, -1);
    if (str == NULL)
	return luaL_error(L, "Missing argument to prompt(..)");

    char input[1024] = { '\0' };
    curs_set(1);
    echo();

    CScreen::clearStatus();
    move(CScreen::height() - 1, 0);
    printw(str);

    timeout(-1000);
    CScreen::readline( input, sizeof(input));
    noecho();
    timeout(1000);

    curs_set(0);

    CScreen::clearStatus();
    lua_pushstring(L, strdup(input));
    return 1;
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

    if (cur < 0)
	cur = 0;

    global->set_selected_folder(cur);
    return (0);
}


/**
 * scroll up/down the message list.
 */
int scroll_index_down(lua_State * L)
{
    int step = lua_tonumber(L, -1);

    CGlobal *global = CGlobal::Instance();

    int cur = global->get_selected_message();
    cur += step;

    global->set_selected_message(cur);

    return 0;
}


/**
 * Scroll the index list up.
 */
int scroll_index_up(lua_State * L)
{
    int step = lua_tonumber(L, -1);

    CGlobal *global = CGlobal::Instance();
    int cur = global->get_selected_message();
    cur -= step;

    if (cur < 0)
	cur = 0;

    global->set_selected_message(cur);
    return (0);
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
   * get the current folders.
   */
    CGlobal *global = CGlobal::Instance();
    std::vector < CMaildir > display = global->get_folders();
    int max = display.size();
    int selected = global->get_selected_folder();

    int i = selected + 1;

    while (i != selected) {
	if (i >= max)
	    break;

	CMaildir cur = display[i];
	if (strstr(cur.path().c_str(), str) != NULL) {
	    global->set_selected_folder(i);
	    break;
	}
	i += 1;

	if (i >= max)
	    i = 0;
    }
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
    std::vector < CMaildir > display = global->get_folders();
    int selected = global->get_selected_folder();

    CMaildir x = display[selected];
    lua_pushstring(L, x.path().c_str());
    return 1;
}


/**
 * Search for the next message matching the pattern.
 */
int scroll_index_to(lua_State * L)
{
    const char *str = lua_tostring(L, -1);

    if (str == NULL)
	return luaL_error(L, "Missing argument to scroll_index_to(..)");

  /**
   * get the current messages
   */
    CGlobal *global = CGlobal::Instance();
    std::vector < CMessage > messages = global->get_messages();
    int max = messages.size();
    int selected = global->get_selected_message();

    int i = selected + 1;

    while (i != selected) {
	if (i >= max)
	    break;

	CMessage cur = messages[i];
	std::string format = cur.format();
	if (strstr(format.c_str(), str) != NULL) {
	    global->set_selected_message(i);
	    break;
	}
	i += 1;

	if (i >= max)
	    i = 0;
    }
    return 0;
}


/**
 * Get the currently selected folders.
 */
int selected_folders(lua_State * L)
{
  CGlobal *global = CGlobal::Instance();
  std::vector<std::string> selected = global->get_selected_folders();
  std::vector<std::string>::iterator it;

  /**
   * Create the table.
   */
  lua_newtable(L);

  int i = 1;
  for (it = selected.begin(); it != selected.end(); ++it) {
    lua_pushnumber(L,i);
    lua_pushstring(L,(*it).c_str());
    lua_settable(L,-3);
    i++;
  }

  return 1;
}


/**
 * Clear all currently selected folders.
 */
int clear_selected_folders(lua_State * L)
{
    CGlobal *global = CGlobal::Instance();
    global->unset_folders();
    global->set_selected_message(0);
    return 0;
}


/**
 * Add the given folder to the selected set.
 */
int add_selected_folder(lua_State * L)
{
  /**
   * get the optional argument.
   */
    const char *str = lua_tostring(L, -1);

    CGlobal *global = CGlobal::Instance();
    CLua    *lua    = CLua::Instance();

  /**
   * default to the current folder.
   */
    if (str == NULL) {
	int selected = global->get_selected_folder();
	std::vector < CMaildir > display = global->get_folders();

	CMaildir x = display[selected];
	global->add_folder(x.path().c_str());
        lua->execute("on_select_folder(\"" + x.path() + "\");");
    } else {
	global->add_folder(std::string(str));
        lua->execute("on_select_folder(\"" + std::string(str) + "\");");
    }

    global->set_selected_message(0);
    return (0);
}


/**
 * Remove all currently selected folders.  Add single new one.
 */
int set_selected_folder(lua_State * L)
{
  /**
   * get the optional argument.
   */
    const char *str = lua_tostring(L, -1);

    CGlobal *global = CGlobal::Instance();
    global->unset_folders();

  /**
   * default to the current folder.
   */
    if (str == NULL) {
	std::vector < CMaildir > display = global->get_folders();
	int selected = global->get_selected_folder();

	CMaildir x = display[selected];
	global->add_folder(x.path().c_str());
        CLua    *lua    = CLua::Instance();
        lua->execute("on_select_folder(\"" + x.path() + "\");");

    } else {
	global->add_folder(std::string(str));
        CLua    *lua    = CLua::Instance();
        lua->execute("on_select_folder(\"" + std::string(str) + "\");");

    }
    return (0);
}


/**
 * Toggle the selection state of the currently selected folder.
 */
int toggle_selected_folder(lua_State * L)
{
  /**
   * get the optional argument.
   */
    const char *str = lua_tostring(L, -1);
    CGlobal *global = CGlobal::Instance();
    std::vector < std::string > sfolders = global->get_selected_folders();

  /**
   * default to the current folder.
   */
    std::string toggle;

    if (str == NULL) {
	std::vector < CMaildir > display = global->get_folders();
	int selected = global->get_selected_folder();
	CMaildir x = display[selected];
	toggle = x.path();
    } else {
	toggle = std::string(str);
    }
    CLua    *lua    = CLua::Instance();
    lua->execute("on_select_folder(\"" + std::string(toggle) + "\");");

    if (std::find(sfolders.begin(), sfolders.end(), toggle) != sfolders.end()) {
	global->remove_folder(toggle);
    } else {
	global->add_folder(toggle);
    }

    return (0);
}


/**
 * Compose a new mail.
 */
int compose(lua_State * L)
{

  char filename[] = "/tmp/mytemp.XXXXXX";
  int fd = mkstemp(filename);

  if (fd == -1)
    return luaL_error(L, "Failed to create a temporary file");



  write(fd, "To: \n", strlen( "To: \n"));
  write(fd, "Subject: New mail\n", strlen( "Subject: New mail\n" ) );
  write(fd, "From: \n" , strlen( "From: \n" ) );
  write(fd, "\n\n", 2 );
  write(fd, "....\n", strlen("....\n" ) );
  close(fd);

    /**
     * Save the current state of the TTY
     */
    refresh();
    def_prog_mode();
    endwin();

    /* Run the edito */
    std::string cmd = "vim ";
    cmd += filename;
    system(cmd.c_str());

    unlink( filename );

    /**
     * Reset + redraw
     */
    reset_prog_mode();
    refresh();

    CLua *lua = CLua::Instance();
    lua->execute( "msg(\"Mail sending should happen here, after a prompt\");" );
    return 0;
}

/**
 * Set the default from address.
 */
int from(lua_State * L)
{
    const char *str = lua_tostring(L, -1);
    CGlobal *g = CGlobal::Instance();

    if (str != NULL)
	g->set_default_from(new std::string(str));

    std::string * s = g->get_default_from();
    lua_pushstring(L, s->c_str());
    return 1;
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
