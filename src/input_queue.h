/*
 * input_queue.h - Process input, via a queue or a keyboard read.
 *
 * This file is part of lumail: http://lumail.org/
 *
 * Copyright (c) 2013-2014 by Steve Kemp.  All rights reserved.
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

#include <string>
#include <vector>

#include "singleton.h"


/**
 * This is a Singleton class which is used for all text-input.
 *
 * The intent is that most of the time we'll call the curses `getch()`
 * function, but we can fake input by reading from an internal buffer
 * instead if we need to.
 *
 * This will allow interesting automation.
 *
 */
class CInputQueue : public Singleton<CInputQueue>
{

public:

    /**
     * Add faux input to the internal queue.
     */
    void add_input(std::string txt);

    /**
     * Return the next input from our faux input queue, or failing
     * that poll for keyboard input with ncurses.
     */
    int get_input();

    /**
     * Is there more input pending in our faux input-buffer?
     */
    bool has_pending_input();

public:

    /**
     * Constructor.
     */
    CInputQueue();

private:


    /**
     * The faux input-buffer we read from.
     */
    std::string m_queue;

};
