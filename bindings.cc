/**
 * bindings.cc - Bindings for all fucntions callable from Lua.
 */

#include <cstdlib>
#include <iostream>
#include <fstream>
#include <string.h>
#include "lua.h"


int my_function(lua_State * L)
{
    int argc = lua_gettop(L);

    std::cerr << "-- my_function() called with " << argc << " arguments:" << std::endl;

    for (int n = 1; n <= argc; ++n) {
	std::cerr << "-- argument " << n << ": "
	    << lua_tostring(L, n) << std::endl;
    }

    lua_pushnumber(L, 123);	// return value
    return 1;			// number of return values
}

int set_mode(lua_State * L)
{
  const char *str = lua_tostring (L, -1);

  if (str == NULL)
    return luaL_error(L, "Missing argument to set_mode(..)" );

  if ( ( strcmp( str, "index" ) != 0 ) &&
       ( strcmp( str, "maildir" ) != 0 ) &&
       ( strcmp( str, "message" ) != 0 ) )
    return luaL_error(L, "Valid modes are: 'index', 'maildir', & 'message'." );

  /** TODO: Set the mode */
  return 0;
}

int get_mode(lua_State *L)
{
  lua_pushstring(L, "the mode is .." );
  return 1;
}

int exit( lua_State *L)
{
  exit(0);
  return 0;
}
