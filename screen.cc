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
 * Setup the curses/screen.
 */
void CScreen::Init()
{

  initscr ();

  /**
   * Make sure we have colours.
   */
  if (!has_colors () || (start_color () != OK))
    {
      endwin();
      std::cerr << "We don't have the required colour support available." << std::endl;
      exit (1);
    }

  keypad (stdscr, TRUE);
  crmode ();
  noecho ();
  curs_set (0);
  timeout (1000);
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
