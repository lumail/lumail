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
#include <sys/ioctl.h>

#include "screen.h"
#include "demo_view.h"


/**
 * Constructor.  NOP.
 */
CDemoView::CDemoView()
{
}

/**
 * Destructor.  NOP.
 */
CDemoView::~CDemoView()
{
}

/**
 * This is the virtual function which is called to refresh the display
 * when the global.mode == "demo"
 */
void CDemoView::draw()
{
    mvprintw(10, 10, "Hello World - This is 'demo' mode");
    mvprintw(12, 10, "Random stars are added, when idle.");

    for (std::vector < DemoStars * >::iterator it = m_stars.begin();
            it != m_stars.end(); ++it)
    {
        DemoStars *cur = (*it);

        wattron(stdscr, cur->c);
        mvprintw(cur->x, cur->y, "*");
        wattroff(stdscr, cur->c);

    }
}


/**
 * Called when things are idle.  NOP.
 */
void CDemoView::on_idle()
{
    struct winsize w;
    ioctl(0, TIOCGWINSZ, &w);

    /**
    * Add a new star.
    */
    DemoStars *add = (DemoStars *)malloc(sizeof(DemoStars));
    add->x = rand() % w.ws_col + 1;
    add->y = rand() % w.ws_row + 1;
    add->c = rand() % 8 + 1;

    m_stars.push_back(add);
}
