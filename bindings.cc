/**
 * bindings.cc - Bindings for all fucntions callable from Lua.
 */

#include <cstdlib>
#include <iostream>
#include <fstream>
#include <string.h>
#include <ncurses.h>

#include "global.h"
#include "screen.h"
#include "lua.h"


/**
 * Set the global mode for lumail.
 */
int set_mode(lua_State * L)
{
  const char *str = lua_tostring (L, -1);

  if (str == NULL)
    return luaL_error(L, "Missing argument to set_mode(..)" );

  if ( ( strcmp( str, "index" ) != 0 ) &&
       ( strcmp( str, "maildir" ) != 0 ) &&
       ( strcmp( str, "message" ) != 0 ) )
    return luaL_error(L, "Valid modes are: 'index', 'maildir', & 'message'." );


  CGlobal *g = CGlobal::Instance();
  g->set_mode( new std::string( str ) );
  return 0;
}


/**
 * Get the global lumail mode.
 */
int get_mode(lua_State *L)
{
  CGlobal *g = CGlobal::Instance();
  std::string *s = g->get_mode();
  lua_pushstring(L, s->c_str() );
  return 1;
}


/**
 * Exit the program.
 */
int exit( lua_State *L)
{
  endwin();
  exit(0);
  return 0;
}


/**
 * Write a message to the status-bar.
 */
int msg( lua_State *L)
{
  const char *str = lua_tostring (L, -1);

  if (str == NULL)
    return luaL_error(L, "Missing argument to msg(..)" );

  move( CScreen::height() -1, 0 );
  printw("%s", str);
  return 0;
}

/**
 * Prompt for input.
 */
int prompt( lua_State *L)
{
  /**
   * Get the prompt string.
   */
  const char *str = lua_tostring (L, -1);
  if (str == NULL)
    return luaL_error(L, "Missing argument to prompt(..)" );


  char input[1024] = { '\0' };

  move( CScreen::height() -1, 0 );

  curs_set (1);
  echo ();

  for (int i = 0; i < CScreen::width(); i++)
    printw (" ");

  move( CScreen::height() -1, 0 );
  printw (str);
  timeout (-1000);

  getstr (input);

  noecho ();
  timeout (1000);

  curs_set (0);
  move( CScreen::height() -1, 0 );
  for (int i = 0; i < CScreen::width(); i++)
    printw (" ");

  lua_pushstring(L, strdup(input) );
  return 1;
}
