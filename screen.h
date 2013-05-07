/**
 * screen.h - Utility functions related to the screen size.
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
#include "maildir.h"

/**
 * Class contains only static methods relating to the screen dimensions.
 */
class CScreen {

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
    void Init();

  /**
   * Return the width of the screen.
   */
    static int width();

  /**
   * Return the height of the screen.
   */
    static int height();

  /**
   * Clear the status-line of the screen.
   */
    static void clearStatus();

 private:
  /**
   * Per-mode drawing primitives.
   */
    void drawMaildir();
    void drawIndex();
    void drawMessage();

};

#endif				/* _screen_h_ */
