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
#include <stdlib.h>
#include <ncurses.h>
#include <unistd.h>


#include "maildir.h"
#include "lua.h"
#include "global.h"
#include "screen.h"



/**
 * Get, or set, the maildir-prefix
 */
int maildir_prefix(lua_State * L)
{
    const char *str = lua_tostring(L, -1);
    CGlobal *g = CGlobal::Instance();

    if (str != NULL)
    {
        if ( !CMaildir::is_directory( str ) )
            return luaL_error(L, "The specified prefix is not a Maildir" );

	g->set_maildir_prefix(new std::string(str));
    }

    std::string * s = g->get_maildir_prefix();
    lua_pushstring(L, s->c_str());
    return 1;
}


/**
 * Get, or set, the index-format
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
 * Get, or set, the global lumail mode.
 */
int global_mode(lua_State * L)
{
    CGlobal *g = CGlobal::Instance();

    /**
     * get the argument, and if we have one set it.
     */
    const char *str = lua_tostring(L, -1);
    if (str != NULL)
    {
        std::string *mode = new std::string(str);
        std::transform(mode->begin(), mode->end(), mode->begin(), tolower);
        g->set_mode(mode);
    }

    /**
     * Return the current/updated value.
     */
    std::string * s = g->get_mode();
    lua_pushstring(L, s->c_str());
    return 1;
}


/**
 * Get, or set, the maildir limit.
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
 * Get, or set, the index limit.
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
     * Update the selected mesages.
     */
    g->update_messages();

    /**
     * Return the current/updated value.
     */
    std::string * s = g->get_index_limit();
    lua_pushstring(L, s->c_str());
    return 1;
}


/**
 * Clear the screen; but not the prompt.
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
 * Redraw the display.
 */
int refresh_display(lua_State * L)
{
    clear();
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
    lua->call_function("on_exit");

    exit(0);
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

    CScreen::clear_status();
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

    CScreen::clear_status();
    move(CScreen::height() - 1, 0);
    printw(str);

    timeout(-1000);
    CScreen::readline( input, sizeof(input));
    noecho();
    timeout(1000);

    curs_set(0);

    CScreen::clear_status();
    lua_pushstring(L, strdup(input));
    return 1;
}


/**
 * Prompt for "y/n".
 */
int prompt_yn(lua_State * L)
{
    const char *def_prompt = "y/n?>";

    /**
     * Get the prompt string.
     */
    const char *str = lua_tostring(L, -1);
    if (str == NULL)
	str = def_prompt;

    echo();

    move(CScreen::height() - 1, 0);
    printw(str);
    timeout(-1000);

    while (true)
    {
	char key = getch();
        if ( key == 'y' || key == 'Y' )
        {
            lua_pushinteger(L, 1 );
            break;
        }
        if ( key == 'n' || key == 'N' )
        {
            lua_pushinteger(L, 0 );
            break;
        }
    }
    noecho();
    curs_set(0);
    timeout(1000);

    CScreen::clear_status();
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
 * Jump to the given message.
 */
int jump_index_to(lua_State * L)
{
    int offset = lua_tonumber(L, -1);

    CGlobal *global = CGlobal::Instance();
    global->set_selected_message(offset);
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
	if (strstr(cur.path().c_str(), str) != NULL)
        {
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
 * Count the visible maildir folders.
 */
int count_maildirs(lua_State *L)
{
    CGlobal *global = CGlobal::Instance();

    std::vector<CMaildir> folders = global->get_folders();
    lua_pushinteger(L, folders.size() );
    return 1;
}

/**
 * Get the names of all currently visible maildirs
 */
int current_maildirs(lua_State *L)
{
    CGlobal *global = CGlobal::Instance();
    std::vector < CMaildir > display = global->get_folders();
    std::vector<CMaildir>::iterator it;

    /**
     * Create the table.
     */
    lua_newtable(L);

    int i = 1;
    for (it = display.begin(); it != display.end(); ++it)
    {
        lua_pushnumber(L,i);
        lua_pushstring(L,(*it).path().c_str());
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
     * Get all messages from the currently selected maildirs.
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
     */
    if ( ( count < 1 ) || selected > count )
    {
        lua_pushstring(L,"" );
        return 1;
    }

    /**
     * Push the path.
     */
    CMessage *cur = messages->at(selected);
    lua_pushstring(L, cur->path().c_str() );
    return 1;
}



/**
 * Is the named/current message new?
 */
int is_new(lua_State * L)
{
  /**
   * See if we were passed a path.
   */
    const char *str = lua_tostring(L, -1);
    if ( str != NULL )
    {
        CMessage cur( str );
        if ( cur.is_new() )
            lua_pushinteger(L,1);
        else
            lua_pushinteger(L,0);

        return 1;
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
     */
    if ( ( count < 1 ) || selected > count )
    {
        lua_pushinteger(L, 0);
        return 1;
    }

    /**
     * Get the value.
     */
    CMessage *cur = messages->at(selected);
    if ( cur->is_new() )
        lua_pushinteger(L,1);
    else
        lua_pushinteger(L,0);

    return 1;
}



/**
 * Mark the message as read.
 */
int mark_read(lua_State * L)
{
  /**
   * See if we were passed a path.
   */
  const char *str = lua_tostring(L, -1);
  if ( str != NULL )
  {
      CMessage cur( str );
      cur.mark_read();
      return 0;
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
   */
  if ( ( count < 1 ) || selected > count )
      return 0;

  /**
   * Mark read..
   */
  CMessage *cur = messages->at(selected);
  cur->mark_read();

  return 0;
}



/**
 * Mark the message as new.
 */
int mark_new(lua_State * L)
{
    /**
     * See if we were passed a path.
     */
    const char *str = lua_tostring(L, -1);
    if ( str != NULL )
    {
        CMessage cur( str );
        cur.mark_new();
        return 0;
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
     */
    if ( ( count < 1 ) || selected > count )
        return 0;

    /**
     * Mark the message
     */
    CMessage *cur = messages->at(selected);
    cur->mark_new();
    return( 0 );
}


/**
 * Delete a message.
 */
int delete_message( lua_State *L )
{
    /**
     * See if we were passed a path.
     */
    const char *str = lua_tostring(L, -1);

    CMessage *msg = NULL;
    int allocated = false;

    if ( str != NULL )
    {
        msg = new CMessage( str );
        allocated = true;
    }

    /**
     * Get all messages from the currently selected messages.
     */
    if ( msg == NULL )
    {
        CGlobal *global = CGlobal::Instance();
        std::vector<CMessage *> *messages = global->get_messages();

        /**
         * The number of items we've found, and the currently selected one.
         */
        int count    = messages->size();
        int selected = global->get_selected_message();

        /**
         * No messages?
         */
        if ( ( count < 1 ) || selected > count )
        {
            return 0;
        }

        /**
         * Get the value.
         */
        msg = messages->at(selected);
    }

    /**
     * Got a message ?
     */
    std::string path = msg->path();
    unlink( path.c_str() );

    CLua *lua = CLua::Instance();
    lua->execute( "msg(\"Deleted: " + path + "\");" );

    if ( allocated )
        delete( msg );

    /**
     * Update messages
     */
    CGlobal *global = CGlobal::Instance();
    global->update_messages();

    /**
     * We're done.
     */
    return 0;
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
    std::vector<CMessage *> *messages = global->get_messages();

    /**
     * If we have no messages we're not scrolling anywhere.
     */
    if ( messages == NULL )
        return 0;


    int max      = messages->size();
    int selected = global->get_selected_message();

    int i = selected + 1;

    while (i != selected) {
        if (i >= max)
            break;

        CMessage *cur = messages->at(i);
        std::string format = cur->format();
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
    for (it = selected.begin(); it != selected.end(); ++it)
    {
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
    global->update_messages();
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
    if (str == NULL)
    {
	int selected = global->get_selected_folder();
	std::vector < CMaildir > display = global->get_folders();

        if ( display.size()  == 0 )
            return 0;

	CMaildir x = display[selected];
        global->add_folder(x.path().c_str());
        lua->execute("on_select_folder(\"" + x.path() + "\");");

    }
    else
    {
	global->add_folder(std::string(str));
        lua->execute("on_select_folder(\"" + std::string(str) + "\");");
    }

    global->set_selected_message(0);
    global->update_messages();
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
    if (str == NULL)
    {
	std::vector < CMaildir > display = global->get_folders();
	int selected = global->get_selected_folder();

	CMaildir x = display[selected];
	global->add_folder(x.path().c_str());
        CLua    *lua    = CLua::Instance();
        lua->execute("on_select_folder(\"" + x.path() + "\");");

    }
    else
    {
	global->add_folder(std::string(str));
        CLua    *lua    = CLua::Instance();
        lua->execute("on_select_folder(\"" + std::string(str) + "\");");
    }

    global->update_messages();
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

    if (str == NULL)
    {
	std::vector < CMaildir > display = global->get_folders();
        if ( display.size()  == 0 )
            return 0;


	int selected = global->get_selected_folder();
	CMaildir x = display[selected];
	toggle = x.path();
    }
    else
    {
	toggle = std::string(str);
    }
    CLua    *lua    = CLua::Instance();
    lua->execute("on_select_folder(\"" + std::string(toggle) + "\");");

    if (std::find(sfolders.begin(), sfolders.end(), toggle) != sfolders.end())
    {
	global->remove_folder(toggle);
    }
    else
    {
	global->add_folder(toggle);
    }

    global->update_messages();
    return (0);
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
 * Compose a new mail.
 */
int compose(lua_State * L)
{
    /**
     * Prompt for the recipient
     */
    lua_pushstring(L, "To: " );
    int ret = prompt( L);
    if ( ret != 1 )
    {
        lua_pushstring(L, "Error recieving recipiient" );
        return( msg(L ) );

    }
    const char *recipient = lua_tostring(L,-1);

    /**
     * Prompt for subject.
     */
    lua_pushstring(L, "Subject: " );
    ret = prompt( L);
    if ( ret != 1 )
    {
        lua_pushstring(L, "Error recieving subject" );
        return( msg(L ) );

    }
    const char *subject = lua_tostring(L,-1);



  CGlobal *global = CGlobal::Instance();
  char filename[] = "/tmp/mytemp.XXXXXX";
  int fd = mkstemp(filename);

  if (fd == -1)
      return luaL_error(L, "Failed to create a temporary file.");

  /**
   * TO
   */
  write(fd, "To: ", strlen( "To: "));
  write(fd, recipient, strlen( recipient ));
  write(fd, "\n", 1 );

  /**
   * Subject.
   */
  write(fd, "Subject: ", strlen( "Subject: " ) );
  write(fd, subject, strlen( subject ) );
  write(fd, "\n", 1 );


  /**
   * From
   */
  write(fd, "From: " , strlen( "From: " ) );
  std::string *from = global->get_default_from();
  write(fd, from->c_str(), strlen( from->c_str() ) );
  write(fd, "\n", 1 );

  /**
   * Space
   */
  write(fd, "\n", 1 );

  /**
   * Body
   */
  write(fd, "....\n", strlen("....\n" ) );
  close(fd);

  /**
   * Save the current state of the TTY
   */
  refresh();
  def_prog_mode();
  endwin();

  /* Run the editor */
  std::string cmd = "vim";

  if ( getenv( "EDITOR" ) )
      cmd = getenv( "EDITOR" );

  cmd += " ";
  cmd += filename;
  system(cmd.c_str());

  /**
   * TODO: prompt for y/n
   */

  // get the sendmail path.
  std::string *sendmail = global->get_sendmail_path();

  char buf[4096];
  ssize_t nread;

  // open the file
  FILE *file = fopen( filename, "r" );

  // open the pipe
  FILE *pipe = popen( sendmail->c_str(), "w" );

  // while read file:  send to pipe
  while (nread = fread( buf, 1, sizeof buf, file), nread > 0)
  {
      char *out_ptr = buf;
      ssize_t nwritten;

      do {
          nwritten = fwrite(out_ptr, sizeof buf, nread, pipe);

          if (nwritten >= 0)
          {
              nread -= nwritten;
              out_ptr += nwritten;
          }
      } while (nread > 0);
  }

  // close the pipe
  // close the file.
  pclose( pipe );
  fclose( file );

  /**
   * TODO: Write this to the sent-mail folder.
   */
  unlink( filename );

  /**
   * Reset + redraw
   */
  reset_prog_mode();
  refresh();

  return 0;
}


/**
 * Get, or set, the default from address.
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
 * Get, or set, the sendmail path.
 */
int sendmail_path(lua_State * L)
{
    const char *str = lua_tostring(L, -1);
    CGlobal *g = CGlobal::Instance();

    if (str != NULL)
	g->set_sendmail_path(new std::string(str));

    std::string * s = g->get_sendmail_path();
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
