/**
 * main.cc - Driver/Wrapper for our lumail script.
 */

#include <algorithm>
#include <cstdlib>
#include <curses.h>
#include <iostream>
#include <fstream>
#include <getopt.h>

#include "lua.h"
#include "message.h"
#include "maildir.h"
#include "screen.h"
#include "version.h"


/**
 * Process a single keystroke from the user.
 */
void processKey( char key )
{
}


/**
 * Entry point to our code.
 */
int main(int argc, char *argv[])
{
  /**
   * Global maildirs.
   *
   * TODO: These should be in global.
   */
  std::vector<CMaildir> g_maildirs;

  /**
   * Temporary hack: Populate the vector of known maildirs.
   */
  std::vector<std::string>  f = CMaildir::getFolders( std::string("/home/skx/Maildir") );
  std::vector < std::string >::iterator it;
  for (it = f.begin(); it != f.end(); ++it) {
    g_maildirs.push_back( CMaildir( *it) );
  }

    //
    //   Parse command-line arguments
    //
    int c;

    //
    // Flags set
    //
    bool version = false;
    std::string rcfile = "";

    while (1) {
	static struct option long_options[] = {
	    {"version", no_argument, 0, 'v'},
	    {"rcfile", required_argument, 0, 'r'},
	    {0, 0, 0, 0}
	};

	/* getopt_long stores the option index here. */
	int option_index = 0;

	c = getopt_long(argc, argv, "vr:", long_options, &option_index);

	/* Detect the end of the options. */
	if (c == -1)
	    break;

	switch (c) {
	case 'r':
	    rcfile = optarg;
	    break;
	case 'v':
	    version = true;
	    break;
	case '?':
	    /* getopt_long already printed an error message. */
	    break;

	default:
	    std::cerr << "Unknown argument" << std::endl;
	    return (2);
	}
    }

    if (version) {
	std::cout << "lumail v" << LUMAIL_VERSION << std::endl;
	return 0;
    }


  /**
   * Initialize the screen.
   */
    CScreen screen = CScreen();
    screen.Init();


  /**
   * Create the lua intepreter.
   */
    CLua *lua = CLua::Instance();
    lua->loadFile("/etc/lumail.lua");
    lua->loadFile("./lumail.lua");


  /**
   * If we have any init file specified then load it up too.
   */
    if (!rcfile.empty())
	lua->loadFile(rcfile.c_str());


  /**
   * We're starting, so call the on_start() function.
   */
    lua->callFunction("on_start");

  /**
   * Now enter our event-loop
   */
    while( true ) {
      char key = getch();
      if (key == ERR) {
        /*
         * Timeout - so we go round the loop again.
         */
        lua->callFunction("on_idle");
      }
      else {
        if ( !lua->onKey( key ) ) {
          std::string foo = "msg(\"Unbound key ";
          foo +=  key;
          foo +=  "\")";
          lua->execute( foo );
        }
      }
      screen.refresh_display();
    }

  /**
   * We've been terminated.
   */
    lua->callFunction("exit");
    exit(0);

    return 0;
}
