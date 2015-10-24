/**
 *
 * The main entry-point for lumail2.
 *
 */


#include <iostream>
#include <gmime/gmime.h>
#include <getopt.h>

#include "config.h"
#include "lua.h"
#include "maildir.h"
#include "message.h"
#include "message_part.h"
#include "screen.h"




/**
 * The entry point to our code.
 */
int main (int argc, char *argv[])
{
    /**
     * Initiate mime.
     */
    g_mime_init (0);

    /**
     * Parse command-line arguments
     */
    int c;


    /**
     * Flags/things set by the command-line arguments.
     */
    std::vector < std::string > load;
    bool curses = true;


    while (1)
    {
	static struct option long_options[] = {
	    {"no-curses", no_argument, 0, 'n'},
	    {"load-file", required_argument, 0, 'l'},
	    {0, 0, 0, 0}
	};

	/* getopt_long stores the option index here. */
	int option_index = 0;

	c = getopt_long (argc, argv, "l:n", long_options, &option_index);

	/* Detect the end of the options. */
	if (c == -1)
	    break;

	switch (c)
	{
	case 'l':
	    load.push_back (optarg);
	    break;
	case 'n':
	    curses = false;
	    break;
	}
    }


    /**
     * If we're supposed to use curses then do so.
     */
    if (curses == true)
    {
        CScreen *screen = CScreen::instance ();
	screen->setup ();
    }


    /**
     * Load the named script file(s).
     */
    if (!load.empty ())
    {
      CLua *instance = CLua::Instance();

      for (std::string filename:load)
	{

          instance->load_file( filename );

	}
    }
    else
    {
      if ( curses == true )
        {
          CScreen *screen = CScreen::instance();

          screen->run_main_loop();
        }
    }


    /**
     * If we're using Curses then tear it down.
     */
    if (curses == true)
      {
        CScreen *screen = CScreen::instance ();
	screen->teardown ();
      }


    /**
     * Close GMime.
     */
    g_mime_shutdown ();

    return 0;
}
