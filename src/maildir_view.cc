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

#include <iostream>
#include <cursesw.h>

#include "config.h"
#include "global_state.h"
#include "maildir.h"
#include "maildir_view.h"


/**
 * Constructor.  NOP.
 */
CMaildirView::CMaildirView()
{
}

/**
 * Destructor.  NOP.
 */
CMaildirView::~CMaildirView()
{
}

/**
 * This is the virtual function which is called to refresh the display
 * when the global.mode == "maildir"
 */
void CMaildirView::draw()
{

    /**
     * Get all maildirs.
     */
    CGlobalState *state = CGlobalState::instance();
    std::vector<std::shared_ptr<CMaildir> > maildirs = state->get_maildirs();

    /**
     * Draw them
     */
    int cur = 0;

    CConfig *config = CConfig::instance();
    std::string max = config->get_string("maildir.max");

    if (max.empty())
        max = "0";

    std::string::size_type sz;
    size_t max_message = std::stoi(max, &sz);

    if (max_message < 1)
    {
        mvprintw(10, 10, "This is 'maildir' mode");
        mvprintw(12, 10, "There are no maildirs found");
        return;
    }


    while (cur < (int)max_message)
    {
        /**
         * Get the maildir.
         */
        std::shared_ptr<CMaildir>  m = maildirs.at(cur);

        /**
         * Draw the path - TODO: Call `to_string`.
         */
        mvprintw(cur, 0, "%s", m->path().c_str());
        cur += 1;
    }
}

/**
 * Called when things are idle.  NOP.
 */
void CMaildirView::on_idle()
{
}
