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
    //
    //   Parse command-line arguments
    //
    int c;

    bool version = false;       /* show version */
    std::string rcfile = "";    /* load rc file */
    std::string folder = "";    /* open folder */

    while (1) {
	static struct option long_options[] = {
	    {"version", no_argument, 0, 'v'},
	    {"rcfile", required_argument, 0, 'r'},
	    {"folder", required_argument, 0, 'f'},
	    {0, 0, 0, 0}
	};

	/* getopt_long stores the option index here. */
	int option_index = 0;

	c = getopt_long(argc, argv, "vr:f:", long_options, &option_index);

	/* Detect the end of the options. */
	if (c == -1)
	    break;

	switch (c) {
	case 'r':
	    rcfile = optarg;
	    break;
	case 'f':
	    folder = optarg;
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
     * If we have a starting folder, select it.
     */
    if ( !folder.empty() ) {
      CLua *lua = CLua::Instance();

      if ( CMaildir::isDirectory( folder ) )
        {
          lua->execute( "global_mode( \"index\" );" );
          lua->execute( "maildir_limit( \"all\" );" );
          lua->execute( "clear_selected_folders();");
          lua->execute( "scroll_maildir_to( \"" + folder + "\");" );
          lua->execute( "add_selected_folder()");
          lua->execute( "global_mode( \"index\" );" );
        }
      else{
        lua->execute("msg(\"Startup folder doesn't exist!\");" );
      }
    }

  /**
   * Now enter our event-loop
   */
    while (true) {
	char key = getch();
	if (key == ERR) {
	    /*
	     * Timeout - so we go round the loop again.
	     */
	    lua->callFunction("on_idle");
	} else {
	    if (!lua->onKey(key)) {
		std::string foo = "msg(\"Unbound key ";
		foo += key;
		foo += "\")";
		lua->execute(foo);
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
