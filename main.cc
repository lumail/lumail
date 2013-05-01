/**
 * main.cc - Driver/Wrapper for our lumail script.
 */


#include <iostream>
#include <fstream>

#include "lua.h"

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

  CLua *s = CLua::Instance();
  s->loadFile("./lumail.lua");

  s->callFunction("on_idle");

  std::string * v = s->getGlobal("VERSION");
  cout << "Version is " << *v << endl;
  delete(v);

//
//   while( true )
//        call: on_idle
//        process events
//        process keyboard
//        refresh screen
//   end
//
//   cleanup

  return 0;
}
