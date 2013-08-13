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
    /**
     * Get the user's home-directory for the logfile.
     */
    std::string home = getenv( "HOME" );
    if ( !home.empty() )
    {
        home += "/lumail.log";
        m_logfile = home;
    }
}


/**
 * Set the path to the file we're logging to.
 */
void CDebug::set_logfile( std::string path )
{
    m_logfile = path;
}

/**
 * Get the current date/time-stamp.
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
 *
 * NOTE: The string might be buffered and not hit the disk immediately.
 */
void CDebug::debug( std::string line, bool force)
{
#ifdef LUMAIL_DEBUG

    /**
     * If we don't have a filename return.
     */
    if ( m_logfile.empty() )
        return;

    /**
     * Add the string to the pending list of log-messages
     * which should be written.
     */
    m_pending.push_back( timestamp() + " " + line );


    /**
     * If the pending list of logfile-entries to write is
     * "small" then return, unless we're forcing the write.
     */
    if ( ( m_pending.size() < 50 ) && ( force == false ) )
        return;

    /**
     * Open the file.
     */
    std::fstream fs;
    fs.open ( m_logfile,  std::fstream::out | std::fstream::app);

    /**
     * Write all pending log entries.
     */
    std::vector<std::string>::iterator it;
    for (it = m_pending.begin(); it != m_pending.end(); ++it)
    {
        fs << (*it) << "\n";
    }

    /**
     * Our pending set is now empty.
     */
    m_pending.clear();

    /**
     * Cleanup.
     */
    fs.close();

#endif
}


