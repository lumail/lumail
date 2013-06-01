/**
 * debug.cc - Debug file for logging purposes.
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

#include <fstream>
#include "debug.h"


/**
 * Instance-handle.
 */
CDebug *CDebug::pinstance = NULL;


/**
 * Get access to our singleton-object.
 */
CDebug *CDebug::Instance()
{
    if (!pinstance)
	pinstance = new CDebug;

    return pinstance;
}


/**
 * Constructor - This is private as this class is a singleton.
 */
CDebug::CDebug()
{
}


/**
 * Get the date/time stamp.
 */
std::string CDebug::timestamp()
{
    time_t     now = time(0);
    struct tm  tstruct;
    char       buf[80];
    tstruct = *localtime(&now);

    strftime(buf, sizeof(buf), "%Y-%m-%d %X", &tstruct);

    return buf;
}


/**
 * Add a new string to the log.
 */
void CDebug::debug( std::string line)
{
#ifdef LUMAIL_DEBUG

    std::fstream fs;
    fs.open ("/home/skx/lumail.log",  std::fstream::out | std::fstream::app);

    fs << timestamp() << line << "\n";
    fs.close();

#endif
}


