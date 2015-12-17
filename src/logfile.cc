/*
 * logfile.cc - Simple singleton for logging output messages.
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

#include <assert.h>
#include <fstream>

#include "logfile.h"



/*
 * Constructor - This is private as this class is a singleton.
 */
CLogfile::CLogfile()
{
    m_filename = "";
}

/*
 * Append a new string to the logfile.
 */
void CLogfile::append(std::string entry)
{
    /*
     * Don't add empty entries.
     */
    if (entry.empty())
        return;

    if (! m_filename.empty())
    {
        std::fstream fs;
        fs.open(m_filename,  std::fstream::out | std::fstream::app);
        fs << entry << "\n";
        fs.close();
    }
}

/*
 * Set the file.
 */
void CLogfile::set_file(std::string filename)
{
    /*
     * Save the filename
     */
    m_filename = filename;
}
