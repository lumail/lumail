/**
 * screen.h - Utility functions related to the screen.
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

#ifndef _screen_h_
#define _screen_h_ 1

#include <vector>
#include <unordered_map>
#include "maildir.h"

/**
 * This class contains simple functions relating to the screen-handling.
 */
class CScreen
{

 public:

  /**
   * Constructor.  NOP.
   */
  CScreen();

  /**
   * Destructor.  NOP.
   */
  ~CScreen();

  /**
   * Draw/Refresh the display.
   */
  void refresh_display();

  /**
   * Setup the screen.
   */
  void setup();

  /**
   * Return the width of the screen.
   */
  static int width();

  /**
   * Return the height of the screen.
   */
  static int height();

  /**
   * Clear the main display area, leaving the status-area alone.
   */
  static void clear_main();

  /**
   * Clear the status-line of the screen.
   */
  static void clear_status();

  /**
   * Handle TAB-expansion of an input string.
   * Return memory the caller must free.
   */
  static char *get_completion( const char *input, size_t size, int position );

  /**
   * Read a line of input.
   */
  static void readline( char *buffer, int buflen );

 private:

  /**
   * Per-mode drawing primitives.
   */
  void drawMaildir();
  void drawIndex();
  void drawMessage();

  /**
   * Colour-maps.
   */
  std::unordered_map<std::string, int> m_colours;


};

enum vectorPosition {
    TOP,
    MIDDLE,
    BOTTOM,
    NONE
};

#endif /* _screen_h_ */
