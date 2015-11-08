/*
 * maildir_view.h - Draw a list of Maildirs.
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



#pragma once

#include "screen.h"


/*
 * This is a maildir-view of the screen - it shows lists of *maildirs*.
 *
 * The list is created, and mainted, by lumail2, but the drawing is
 * deferred to Lua.
 */
class CMaildirView: public CViewMode
{

public:
    CMaildirView();
    ~CMaildirView();

    /*
     * Drawing routine - called when the current.mode=="maildir".
     */
    void draw();

    /*
     * Called when things are idle.
     */
    void on_idle();

private:

    /*
     * Get the output of calling `maildir_view`, which is the text we'll display.
     */
    std::vector<std::string> get_text();

};
