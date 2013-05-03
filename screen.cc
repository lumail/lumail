/**
 * screen.cc - Utility functions related to the screen size.
 */

#include <cstdlib>
#include <iostream>
#include <fstream>
#include <sys/ioctl.h>
#include <ncurses.h>
#include "screen.h"


/**
 * Constructor.  NOP.
 */
CScreen::CScreen()
{
}


/**
 * Destructor.  NOP.
 */
CScreen::~CScreen()
{
}


/**
 * Draw a list of folders.
 */
void CScreen::drawMaildir( std::vector<CMaildir> x)
{
  clear();

  int height = CScreen::height();
  int size = x.size();
  for( int i = 0; i < (height -1); i++ )
    {
      move(i, 1 );
      printw("%s - %d:%d", x[i].name().c_str(), i , size);
    }
}


/**
 * Setup the curses/screen.
 */
void CScreen::Init()
{
  /**
   * Setup ncurses.
   */
  initscr();

  /**
   * Make sure we have colours.
   */
    if (!has_colors() || (start_color() != OK)) {
	endwin();
	std::
	    cerr << "We don't have the required colour support available."
	    << std::endl;
	exit(1);
    }

    keypad(stdscr, TRUE);
    crmode();
    noecho();
    curs_set(0);
    timeout(1000);
}


/**
 * Return the width of the screen.
 */
int CScreen::width()
{
    struct winsize w;
    ioctl(0, TIOCGWINSZ, &w);
    return (w.ws_col);
}


/**
 * Return the height of the screen.
 */
int CScreen::height()
{
    struct winsize w;
    ioctl(0, TIOCGWINSZ, &w);
    return (w.ws_row);
}
