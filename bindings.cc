/**
 * bindings.cc - Bindings for all functions callable from Lua.
 */

#include <algorithm>
#include <cstdlib>
#include <iostream>
#include <fstream>
#include <string.h>
#include <ncurses.h>

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
    CGlobal      *g = CGlobal::Instance();

    if (str != NULL)
      g->set_maildir_prefix(new std::string(str));

    std::string * s = g->get_maildir_prefix();
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
      g->set_mode(new std::string( str ));

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
        g->set_maildir_limit(new std::string( str ));

    /**
     * Return the current/updated value.
     */
    std::string * s = g->get_maildir_limit();
    lua_pushstring(L, s->c_str());
    return 1;
}


/**
 * Clear the screen.
 */
int clear(lua_State * L)
{
    erase();
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
    getstr(input);

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
int scroll_maildir_down(lua_State *L){
  int step = lua_tonumber (L, -1);

  CGlobal *global = CGlobal::Instance();

  int cur = global->get_selected_folder();
  cur += step;

  global->set_selected_folder( cur );

  return 0;
}

/**
 * Scroll the maildir list up.
 */
int scroll_maildir_up(lua_State *L) {
  int step = lua_tonumber (L, -1);

  CGlobal *global = CGlobal::Instance();
  int cur = global->get_selected_folder();
  cur -= step;

  if ( cur < 0 )
    cur = 0;

  global->set_selected_folder( cur );
  return( 0 );
}


/**
 * scroll to the folder matching the pattern.
 */
int scroll_maildir_to(lua_State *L)
{
  const char *str = lua_tostring(L, -1);

  if (str == NULL)
    return luaL_error(L, "Missing argument to scroll_maildir_to(..)");

  /**
   * get the current folders.
   */
  CGlobal               *global = CGlobal::Instance();
  std::vector<CMaildir> display = global->get_folders();
  int                       max = display.size();
  int                  selected = global->get_selected_folder();

  int i = selected + 1;

  while( i != selected )
  {
    if ( i >= max )
      break;

    CMaildir cur = display[i];
    if ( strstr(cur.path().c_str(), str ) != NULL ) {
      global->set_selected_folder( i );
      break;
    }
    i += 1;

    if ( i >= max )
      i = 0;
  }
  return 0;
}


/**
 * Get the currently highlighted maildir folder.
 */
int current_maildir(lua_State *L)
{
  /**
   * get the current folders.
   */
  CGlobal               *global = CGlobal::Instance();
  std::vector<CMaildir> display = global->get_folders();
  int                  selected = global->get_selected_folder();

  CMaildir x = display[selected];
  lua_pushstring(L, x.path().c_str());
  return 1;
}

/**
 * folder selection code.
 */
int clear_selected_folders(lua_State * L)
{
  CGlobal *global = CGlobal::Instance();
  global->unset_folders();
  return 0;
}

int add_selected_folder(lua_State * L)
{
  /**
   * get the optional argument.
   */
  const char *str = lua_tostring(L, -1);

  CGlobal *global = CGlobal::Instance();

  /**
   * default to the current folder.
   */
  if (str == NULL) {
    int                  selected = global->get_selected_folder();
    std::vector<CMaildir> display = global->get_folders();

    CMaildir x = display[selected];
    global->add_folder( x.path().c_str());
  }
  else {
    global->add_folder( std::string(str) );
  }
  return( 0 );
}

/**
 * Remove all entries.  Add single new one.
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
    std::vector<CMaildir> display = global->get_folders();
    int                  selected = global->get_selected_folder();

    CMaildir x = display[selected];
    global->add_folder( x.path().c_str());
  }
  else {
    global->add_folder( std::string(str) );
  }
  return( 0 );
}

/**
 * Toggle the current item.
 */
int toggle_selected_folder(lua_State * L)
{
  /**
   * get the optional argument.
   */
  const char *str = lua_tostring(L, -1);
  CGlobal *global = CGlobal::Instance();
  std::vector<std::string> sfolders = global->get_selected_folders();

  /**
   * default to the current folder.
   */
  std::string toggle;

  if (str == NULL) {
    std::vector<CMaildir> display = global->get_folders();
    int                  selected = global->get_selected_folder();
    CMaildir x = display[selected];
    toggle = x.path();
  }
  else {
    toggle = std::string(str) ;
  }

  if ( std::find( sfolders.begin(), sfolders.end(), toggle ) != sfolders.end() ) {
    global->remove_folder( toggle );
  }
  else {
    global->add_folder( toggle );
  }


  return( 0 );
}

