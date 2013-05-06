/**
 * screen.cc - Utility functions related to the screen size.
 */

#include <cstdlib>
#include <iostream>
#include <fstream>
#include <string.h>
#include <sys/ioctl.h>
#include <ncurses.h>
#include "global.h"
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
 * This function will draw the appropriate screen, depending upon our current mode.
 */
void CScreen::refresh_display()
{
  /**
   * Get the current mode.
   */
  CGlobal *g = CGlobal::Instance();
  std::string * s = g->get_mode();

  if ( strcmp( s->c_str(), "maildir" ) == 0 )
    drawMaildir();
  else if ( strcmp( s->c_str(), "index") == 0 )
    drawIndex();
  else if ( strcmp( s->c_str(), "message" ) == 0 )
    drawMessage();
  else
    {
      clear();
      move( 3, 3 );
      printw( "UNKNOWN MODE: '%s'", s->c_str() );
    }
}


/**
 * Draw a list of folders.
 */
void CScreen::drawMaildir()
{

  /**
   * Get all known folders + the current display mode
   */
  CGlobal               *global = CGlobal::Instance();
  std::vector<CMaildir> folders = global->get_all_folders();
  std::vector<CMaildir> display;
  std::string           *filter = global->get_maildir_limit();

  /**
   * Filter the folders to those we can display
   */
  std::vector<CMaildir>::iterator it;
  for (it = folders.begin(); it != folders.end(); ++it)
    {
      CMaildir x = *it;

      if ( strcmp( filter->c_str(), "all") == 0 ){
        display.push_back( x );
      }
      else if ( strcmp( filter->c_str(), "new") == 0 )  {
        if ( x.newMessages() > 0 ) {
          display.push_back( x );
        }
      }
      else {
        std::string  path = x.path();
        if ( path.find( *filter, 0 ) !=std::string::npos ) {
            display.push_back( x );
        }
      }
    }


  /**
   * The number of items we've found, vs. the size of the screen.
   */
  int count    = display.size();
  int height   = CScreen::height();
  int selected = global->get_selected_folder();

  /*
   * Bound the selection.
   */
  if ( selected > count ){
    selected = 0;
    global->set_selected_folder( 0 );
  }


  int row = 0;

  for( row = 0; ((selected < count ) && ( row < count ) &&( row+selected <= count ) && ( row < (height-1))); row++ )
  {
    move(row,2);

    /**
     * First row is the current one.
     */
    if ( row == 0 )
      attron( A_REVERSE);

    std::string path = display[row+selected].path();

    if ( (display[row + selected]).newMessages() > 0 )
	attrset (COLOR_PAIR (1));

    printw("[ ] - %-70s", path.c_str() );

    attrset (COLOR_PAIR (2));

    /**
     * Remove the inverse.
     */
    if ( row == 0 )
      attroff(A_REVERSE);
  }
}



/**
 * Draw the index-mode.
 */
void CScreen::drawIndex()
{
  move(3, 3);
  printw( "Drawing INDEX here ..");
}


/**
 * Draw the message mode.
 */
void CScreen::drawMessage()
{
  move(3, 3);
  printw( "Drawing MESSAGE here ..");
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

  /**
   * We want (red + black) + (white + black)
   */
  init_pair (1, COLOR_RED, COLOR_BLACK);
  init_pair (2, COLOR_WHITE, COLOR_BLACK);

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

/**
 * Clear the status-line of the screen.
 */
void CScreen::clearStatus()
{
  move(CScreen::height() - 1, 0);

  for (int i = 0; i < CScreen::width(); i++)
    printw(" ");

}
