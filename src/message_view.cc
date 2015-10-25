/**
 * $FILENAME - $TITLE
 *
 * This file is part of lumail - http://lumail.org/
 *
 * Copyright (c) 2015 by Steve Kemp.  All rights reserved.
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


#include <cursesw.h>

#include "message_view.h"


/**
 * Constructor.  NOP.
 */
CMessageView::CMessageView()
{
}

/**
 * Destructor.  NOP.
 */
CMessageView::~CMessageView()
{
}

/**
 * This is the virtual function which is called to refresh the display
 * when the global.mode == "index"
 */
void CMessageView::draw()
{
    mvprintw(10, 10, "This is 'message' mode");
    mvprintw(12, 10, "This should draw a single message.");
}
