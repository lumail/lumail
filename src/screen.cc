#include <sys/ioctl.h>
#include <cursesw.h>
#include <iostream>
#include <fstream>
#include <unistd.h>

#include "screen.h"



/**
 * Constructor.  NOP.
 */
CScreen::CScreen ()
{
}

CScreen *
CScreen::instance ()
{
    static CScreen *instance = new CScreen ();
    return (instance);
}

/**
 * Destructor.  NOP.
 */
CScreen::~CScreen ()
{
    teardown ();
}

/**
 * Setup the curses/screen.
 */
void
CScreen::setup ()
{
    /**
     * Setup locale.
     */
    setlocale (LC_CTYPE, "");
    setlocale (LC_ALL, "");

    char e[] = "ESCDELAY=0";
    putenv (e);

    /**
     * Setup ncurses.
     */
    initscr ();

    /**
     * Make sure we have colours.
     */
    if (!has_colors () || (start_color () != OK))
    {
	endwin ();
	std::cerr << "Missing colour support" << std::endl;
	exit (1);
    }

    /* Initialize curses */
    initscr ();
    start_color ();
    raw ();
    cbreak ();
    noecho ();
    keypad (stdscr, TRUE);
    use_default_colors ();
}


/**
 * Shutdown curses.
 */
void
CScreen::teardown ()
{
    endwin ();
}


/**
 * Clear the screen
 */
void
CScreen::clear ()
{
  /**
   * Clear the whole screen.
   */
    int width = CScreen::width ();
    int height = CScreen::height ();

    std::string blank = "";
    while ((int) blank.length () < width)
	blank += " ";

    for (int i = 0; i < height; i++)
    {
	mvprintw (i, 0, "%i%s", i, blank.c_str ());
    }
    doupdate ();
}


/**
 * Return the height of the screen.
 */
int
CScreen::height ()
{
    struct winsize w;
    ioctl (0, TIOCGWINSZ, &w);
    return (w.ws_row);
}


/**
 * Delay for the given period.
 */
void
CScreen::sleep (int seconds)
{
    ::sleep (seconds);
}

/**
 * Return the width of the screen.
 */
int
CScreen::width ()
{
    struct winsize w;
    ioctl (0, TIOCGWINSZ, &w);
    return (w.ws_col);
}
