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
 * Entry point to our code.
 */
int main(int argc, char *argv[])
{
#if 0
    CMaildir steve = CMaildir( "/home/skx/Maildir/.steve.org.uk" );
  std::vector<CMessage> mess = steve.getMessages();
  std::vector < CMessage >::iterator mit;
  for (mit = mess.begin(); mit != mess.end(); ++mit) {
    std::cout << mit->path() << " " << mit->flags() << std::endl;
  }
  exit( -1 );
#endif

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
    //   parse arguments
    //
    int c;

    //
    // Flags set
    //
    bool verbose = false;
    bool version = false;
    std::string rcfile = "";

    while (1) {
	static struct option long_options[] = {
	    {"verbose", no_argument, 0, 'v'},
	    {"version", no_argument, 0, 'V'},
	    {"rcfile", required_argument, 0, 'r'},
	    {0, 0, 0, 0}
	};

	/* getopt_long stores the option index here. */
	int option_index = 0;

	c = getopt_long(argc, argv, "vVr:", long_options, &option_index);

	/* Detect the end of the options. */
	if (c == -1)
	    break;

	switch (c) {
	case 'r':
	    rcfile = optarg;
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
	    std::cerr << "Unknown argument" << std::endl;
	    return (2);
	}
    }

    if (verbose) {
	std::cout << "--verbose" << std::endl;
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
    char key = getch();
    int run = 1;
    while (run) {
	if (key == ERR) {
	    /*
	     * Timeout - so we go round the loop again.
	     */
	    lua->callFunction("on_idle");
	} else {
	    if (key == 'q') {
		run = 0;
	    } else if (key == ':') {
	    /**
             * This is inspirational.
             */
		lua->execute("loadstring(prompt(\":\"))();");
	    } else {
              std::string msg;
              msg = "Unbound key:";
              msg += key;
              lua->execute("loadstring(msg(\"" + msg + "\"))();");
	    }
	}
	key = getch();
#if 0
        screen.drawMaildir( g_maildirs );
#else
        screen.refresh_display();
#endif
    }

  /**
   * We've been terminated.
   */
    lua->callFunction("exit");
    exit(0);

    return 0;
}
