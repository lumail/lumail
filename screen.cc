/**
 * screen.cc - Utility functions related to the screen size.
 *
 * This file is part of lumail: http://lumail.org/
 *
 * Copyright (c) 2013 by Steve Kemp.  All rights reserved.
 *
 **
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; version 2 dated June, 1991, or (at your
 * option) any later version.
 *
 * On Debian GNU/Linux systems, the complete text of version 2 of the GNU
 * General Public License can be found in `/usr/share/common-licenses/GPL-2'
 */

#include <algorithm>
#include <cstdlib>
#include <iostream>
#include <algorithm>
#include <fstream>
#include <string.h>
#include <sys/ioctl.h>
#include <ncurses.h>
#include "lua.h"
#include "global.h"
#include "message.h"
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

    if (strcmp(s->c_str(), "maildir") == 0)
	drawMaildir();
    else if (strcmp(s->c_str(), "index") == 0)
	drawIndex();
    else if (strcmp(s->c_str(), "message") == 0)
	drawMessage();
    else {
      CLua *lua = CLua::Instance();
      lua->execute( "clear();" );
      move(3, 3);
      printw("UNKNOWN MODE: '%s'", s->c_str());
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
    CGlobal *global = CGlobal::Instance();
    std::vector < CMaildir > display = global->get_folders();

  /**
   * The number of items we've found, vs. the size of the screen.
   */
    int count = display.size();
    int height = CScreen::height();
    int selected = global->get_selected_folder();

    /*
     * Bound the selection.
     */
    if (selected >= count) {
	global->set_selected_folder(0);
	selected = 0;
    }

  /**
   * Something here is screwy - segfaults if the number of folders
   * is not less larger than the height of the screen.
   */
    int row = 0;

  /**
   * Selected folders.
   */
    std::vector < std::string > sfolders = global->get_selected_folders();

    for (row = 0; row < (height - 1); row++) {
    /**
     * What we'll output for this row.
     */
	char buf[250] = { '\0' };
        int unread = 0;


    /**
     * The current object.
     */
	CMaildir *cur = NULL;
	if ((row + selected) < count) {
	    cur = &display[row + selected];
            unread = cur->newMessages();
        }
	std::string found = "[ ]";
	if (cur != NULL) {
	    if (std::find(sfolders.begin(), sfolders.end(), cur->path()) !=
		sfolders.end())
		found = "[x]";
	}

    /**
     * First row is the current one.
     */
	if (row == 0)
          attron(A_STANDOUT);

	std::string path = "";

	if (cur != NULL)
	    snprintf(buf, sizeof(buf) - 1, "%s - %-70s", found.c_str(),
		     cur->path().c_str());

	while ((int)strlen(buf) < (CScreen::width() - 3))
	    strcat(buf, " ");

	move(row, 2);

        if ( unread ) {
          if ( row == 0 )
            attrset( COLOR_PAIR(1) |A_REVERSE );
          else
            attrset( COLOR_PAIR(1) );
        }
	printw("%s", buf);

        attrset( COLOR_PAIR(2) );

    /**
     * Remove the inverse.
     */
	if (row == 0)
	    attroff(A_STANDOUT);
    }
}

/**
 * Draw the index-mode.
 */
void CScreen::drawIndex()
{
  /**
   * Get all messages from the currently selected maildirs.
   */
    CGlobal *global = CGlobal::Instance();
    std::vector < CMessage > messages = global->get_messages();

  /**
   * If we have no messages report that.
   */
    if (messages.size() < 1) {
	move(2, 2);
	printw("No messages found");
	return;
    }

  /**
   * The number of items we've found, vs. the size of the screen.
   */
    int count = messages.size();
    int height = CScreen::height();
    int selected = global->get_selected_message();

    /*
     * Bound the selection.
     */
    if (selected >= count) {
	global->set_selected_message(0);
	selected = 0;
    }

  /**
   * OK so we have (at least one) selected maildir and we have messages.
   */
    int row = 0;

    for (row = 0; row < (height - 1); row++) {
    /**
     * What we'll output for this row.
     */
	char buf[250] = { '\0' };

    /**
     * The current object.
     */
	CMessage *cur = NULL;
	if ((row + selected) < count)
	    cur = &messages[row + selected];

        bool unread = false;
        if ( cur != NULL ) {
          std::string flags = cur->flags();
          if ( flags.find( "N" ) != std::string::npos )
            unread = true;
        }

	if ( unread ) {
          if (row == 0)
	    attrset(COLOR_PAIR(1)|A_REVERSE);
          else
	    attrset(COLOR_PAIR(1));
        }
        else {
          if (row == 0)
	    attrset(A_REVERSE);
        }

	std::string path = "";

	if (cur != NULL)
	    snprintf(buf, sizeof(buf) - 1, "%s", (*cur).format().c_str());

    /**
     * Pad.
     */
	while ((int)strlen(buf) < (CScreen::width() - 3))
	    strcat(buf, " ");

    /**
     * Truncate.
     */
	if ((int)strlen(buf) > (CScreen::width() - 3))
	    buf[(CScreen::width() - 3)] = '\0';

	move(row, 2);
	printw("%s", buf);

        attrset( COLOR_PAIR(2) );

    /**
     * Remove the inverse.
     */
	if (row == 0)
	    attroff(A_REVERSE);
    }

}

/**
 * Draw the message mode.
 */
void CScreen::drawMessage()
{
  /**
   * Get all messages from the currently selected maildirs.
   */
  CGlobal *global = CGlobal::Instance();
  std::vector < CMessage > messages = global->get_messages();

  /**
   * The number of items we've found, vs. the size of the screen.
   */
  int count = messages.size();
  int selected = global->get_selected_message();

  /**
   * Screen width, used to truncate long strings.
   */
  int width = CScreen::width();

  /**
   * Bound the selection.
   */
  if (selected >= count) {
    global->set_selected_message(0);
    selected = 0;
  }

  CMessage *cur = NULL;
  if ((selected) < count)
    cur = &messages[ selected];
  else
    {
      clear();
      move(3,3);
      printw("No selected message?!");
      return;
    }

  /**
   * Now we have a message - display it.
   *
   * TODO: Do this properly.
   */
  CLua *lua = CLua::Instance();
  lua->execute( "clear();" );

  move(0,2);
  printw("From: %s", cur->from().substr(0, width - 12 ).c_str() );
  move(1,2);
  printw("Subject: %s", cur->subject().substr(0, width-12).c_str() );
  move(2,2);
  printw("Date: %s", cur->date().substr(0, width-12).c_str() );
  move(3,2);
  printw("To: %s", cur->to().substr(0, width-12).c_str());

  /**
   * Now draw the body.
   */
  std::vector<std::string> body = cur->body();

  /**
   * How many lines to draw?
   */
  int max = std::min((int)body.size(), CScreen::height()-6);

  for( int i = 0; i < max; i++ )
    {
      move( i + 5, 0 );
      printw( "%s", body[i].c_str() );
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
	std::cerr << "We don't have the required colour support available."
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
    init_pair(1, COLOR_RED, COLOR_BLACK);
    init_pair(2, COLOR_WHITE, COLOR_BLACK);

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


/* Read up to buflen-1 characters into `buffer`.
 * A terminating '\0' character is added after the input.  */
void CScreen::readline(char *buffer, int buflen)
{
  int old_curs = curs_set(1);
  int pos = 0;
  int len = 0;
  int x, y;
  static char *prev = NULL;

  getyx(stdscr, y, x);
  for (;;) {
    int c;

    buffer[len] = ' ';
    mvaddnstr(y, x, buffer, len+1);
    for( int padding = len; padding < CScreen::width(); padding++ )
      {printw(" ");}

    move(y, x+pos);
    c = getch();

    if (c == KEY_ENTER || c == '\n' || c == '\r') {
      break;
    } else if (isprint(c)) {
      if (pos < buflen-1) {
        memmove(buffer+pos+1, buffer+pos, len-pos);
        buffer[pos++] = c;
        len += 1;
      } else {
        beep();
      }
    } else if (c == KEY_LEFT) {
      if (pos > 0) pos -= 1; else beep();
    } else if ( c == KEY_UP ) {
      if ( prev != NULL )
        {
          strcpy( buffer, prev );
          pos = len = strlen(buffer);
        }
      else {
        beep();
      }

    }
    else if (c == KEY_RIGHT) {
      if (pos < len) pos += 1; else beep();
    } else if (c == KEY_BACKSPACE) {
      if (pos > 0) {
        memmove(buffer+pos-1, buffer+pos, len-pos);
        pos -= 1;
        len -= 1;
      } else {
        beep();
      }
    } else if (c == KEY_DC) {
      if (pos < len) {
        memmove(buffer+pos, buffer+pos+1, len-pos-1);
        len -= 1;
      } else {
        beep();
      }
    } else {
      beep();
    }
  }
  buffer[len] = '\0';
  if (old_curs != ERR) curs_set(old_curs);

  if ( prev != NULL )
    free( prev );
  prev = strdup( buffer);
}
