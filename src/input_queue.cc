/*
 * input_queue.cc - Process input, via a queue or a keyboard read.
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

#include <string>
#include <cursesw.h>

#include "input_queue.h"



/*
 * Constructor - This is private as this class is a singleton.
 */
CInputQueue::CInputQueue()
{
    m_queue = "";
}


/*
 * Add a new string to the faux input-buffer.
 */
void CInputQueue::add_input(std::string text)
{
    m_queue += text;
}

int CInputQueue::get_input()
{
    /*
     * No queued input?  Return via curses.
     */
    if (m_queue.empty())
    {
        return (getch());
    }

    /*
     * Remove the first character from our queue, and return it.
     */
    int tmp = m_queue.at(0);
    m_queue = m_queue.substr(1);

    return tmp;
}
