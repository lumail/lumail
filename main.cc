/**
 * main.cc - Driver/Wrapper for our lumail script.
 */


#include <iostream>
#include <fstream>

#include "lua.h"
#include "maildir.h"

using namespace std;



/**
 * Some C-code called from Lua.
 */
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


/**
 * Entry point to our code.
 */
int main( int argc, char *argv[] )
{
//   parse arguments
//
//   setup console
//
//   load lua environment

  CLua *lua = CLua::Instance();
  lua->loadFile("./lumail.lua");

  lua->callFunction("on_idle");

  std::string * v = lua->getGlobal("VERSION");
  cout << "Version is " << *v << endl;
  delete(v);

  cout <<endl;
  if ( CMaildir::isMaildir( "/home/skx/Maildir" ) )
    cout << "\t~/Maildir is OK" << endl;
  if ( CMaildir::isMaildir( "/home/skx/Maildir/.steve.org.uk" ) )
    cout << "\t~/Maildir/.steve.org.uk is OK" << endl;

//
//   while( true )
//        call: on_idle
  while( true )
    {

      lua->callFunction("on_idle");

//        process events
//        process keyboard
//        refresh screen
      sleep( 1);


//   end
    }
//
//   cleanup

  return 0;
}
