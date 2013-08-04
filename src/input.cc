/**
 * input.cc - A faux input-buffer.
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


#include <cstdlib>
#include <cursesw.h>
#include "input.h"


/**
 * Instance-handle.
 */
CInput *CInput::pinstance = NULL;


/**
 * Get access to our singleton-object.
 */
CInput *CInput::Instance()
{
    if (!pinstance)
        pinstance = new CInput;

    return pinstance;
}


/**
 * Constructor - This is private as this class is a singleton.
 */
CInput::CInput()
{
    /**
     * Our faux input buffer is empty.
     */
    m_pending = "";
    m_offset = 0;
}


/**
 * Enqueue some input to the input buffer.
 */
void CInput::add( std::string input )
{
    m_pending = input;
    m_offset  = 0;
}


/**
 * Get a character from either our faux buffer, or via curses.
 */
int CInput::get_char()
{
    /**
     * If we have pending history - return the next character from it.
     */
    if ( !m_pending.empty() )
    {
        /**
         * If we've not walked off the end.
         */
        if ( m_offset < m_pending.size() )
        {
            /**
             * Get the character and return it,
             * updating our current-offset.
             */
            int c = m_pending.at( m_offset );
            m_offset += 1;

            return( c );
        }
    }

    /**
     * Otherwise defer to ncurses.
     */
    return( getch() );
}
