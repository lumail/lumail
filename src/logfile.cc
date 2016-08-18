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
#include <chrono>
#include <fstream>
#include <iomanip>

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

        tm localTime;
        std::chrono::system_clock::time_point t = std::chrono::system_clock::now();
        time_t now = std::chrono::system_clock::to_time_t(t);
        localtime_r(&now, &localTime);

        const std::chrono::duration<double> tse = t.time_since_epoch();
        std::chrono::seconds::rep milliseconds = std::chrono::duration_cast<std::chrono::milliseconds>(tse).count() % 1000;

        std::fstream fs;
        fs.open(m_filename,  std::fstream::out | std::fstream::app);

        fs
                << std::setfill('0') << std::setw(2) << localTime.tm_mday << '/'
                << std::setfill('0') << std::setw(2) << (localTime.tm_mon + 1) << '/'
                << std::setfill('0') << std::setw(4) << (1900 + localTime.tm_year) << ' '
                << std::setfill('0') << std::setw(2) << localTime.tm_hour << ':'
                << std::setfill('0') << std::setw(2) << localTime.tm_min << ':'
                << std::setfill('0') << std::setw(2) << localTime.tm_sec << '.'
                << std::setfill('0') << std::setw(3) << milliseconds
                << " ";

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
