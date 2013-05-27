/**
 * history.cc - History wrapper for prompt-input
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


#include "history.h"

/**
 * Instance-handle.
 */
CHistory *CHistory::pinstance = NULL;

/**
 * Get access to our singleton-object.
 */
CHistory *CHistory::Instance()
{
    if (!pinstance)
	pinstance = new CHistory;

    return pinstance;
}

/**
 * Constructor - This is private as this class is a singleton.
 */
CHistory::CHistory()
{
}


int CHistory::size()
{
    return( m_map.size() );
}

std::string CHistory::at( int offset )
{
    return( m_map.at( offset ) );
}

void CHistory::add( std::string entry)
{
    m_map.push_back(entry);
}
