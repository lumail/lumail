/**
 * main.cc - Driver/Wrapper for our lumail script.
 */


#include <iostream>
#include <fstream>
#include <getopt.h>

#include "lua.h"
#include "maildir.h"
#include "version.h"

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

  //
  //   parse arguments
  //
  int c;

  //
  // Flags set
  //
  bool verbose = false;
  bool version = false;

  while (1)
    {
      static struct option long_options[] =
        {
          {"verbose", no_argument,  0, 'v'},
          {"version", no_argument,  0, 'V'},
          {"rcfile", required_argument, 0, 'r'},
          {0, 0, 0, 0}
        };

      /* getopt_long stores the option index here. */
      int option_index = 0;

      c = getopt_long (argc, argv, "vVr:", long_options, &option_index);

      /* Detect the end of the options. */
      if (c == -1)
        break;

      switch (c)
        {
        case 'r':
          printf ("option --rcfile with file `%s'\n", optarg);
          break;
        case 'v':
          verbose = true;
          break;
        case 'V':
          version = true;
          break;
        case '?':
          /* getopt_long already printed an error message. */
          break;

        default:
          cerr << "Unknown argument" << endl;
          return(2);
        }
    }

  if ( verbose )
    {
      cout << "--verbose" << endl;
    }
  if ( version )
    {
      cout << "lumail v" << LUMAIL_VERSION << endl;
      return 0;
    }
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
