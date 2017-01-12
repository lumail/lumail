/*
 * logger.cc - Simple singleton for logging messages.
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

#include <chrono>
#include <fstream>
#include <iomanip>
#include <stdarg.h>
#include <string.h>

#include "logger.h"
#include "util.h"



/*
 * Constructor - This is private as this class is a singleton.
 */
CLogger::CLogger()
{
    m_level = "";
    m_path  = "";
}

/*
 * Log a message, if the level includes it.
 */
void CLogger::log(const char *level, const char *fmt,  ...)
{
    if (m_level.empty())
        return;

    if (m_path.empty())
        return;

    /*
     * Only log if the level matches, or is all.
     */
    std::vector<std::string> levels;
    levels = split(m_level, '|');

    /*
     * Should we log?
     */
    bool log = false;

    /*
     * Is the log-level "all", or otherwise a match for the
     * level?
     */
    for (std::vector<std::string>::iterator it = levels.begin(); it != levels.end() ; ++it)
    {
        std::string opt = (*it);

        if ((opt == "all") || (opt == level))
            log = true;
    }

    if (log == false)
        return;


    char buffer[1024];
    va_list args;
    va_start(args, fmt);
    memset(buffer, '\0', sizeof(buffer));
    vsnprintf(buffer, sizeof(buffer) - 1, fmt, args);
    va_end(args);


    tm localTime;
    std::chrono::system_clock::time_point t = std::chrono::system_clock::now();
    time_t now = std::chrono::system_clock::to_time_t(t);
    localtime_r(&now, &localTime);

    const std::chrono::duration<double> tse = t.time_since_epoch();
    std::chrono::seconds::rep milliseconds = std::chrono::duration_cast<std::chrono::milliseconds>(tse).count() % 1000;

    std::fstream fs;
    fs.open(m_path,  std::fstream::out | std::fstream::app);

    fs
            << std::setfill('0') << std::setw(2) << localTime.tm_mday << '/'
            << std::setfill('0') << std::setw(2) << (localTime.tm_mon + 1) << '/'
            << std::setfill('0') << std::setw(4) << (1900 + localTime.tm_year) << ' '
            << std::setfill('0') << std::setw(2) << localTime.tm_hour << ':'
            << std::setfill('0') << std::setw(2) << localTime.tm_min << ':'
            << std::setfill('0') << std::setw(2) << localTime.tm_sec << '.'
            << std::setfill('0') << std::setw(3) << milliseconds
            << " <"
            << level
            << "> ";

    fs << buffer << "\n";
    fs.close();
}

/*
 * Get the log-level
 */
std::string CLogger::get_level()
{
    return(m_level);
}

/*
 * Change the log-level
 */
void CLogger::set_level(std::string level)
{
    m_level = level;
}

/*
 * Change the log-file.
 */
void CLogger::set_path(std::string path)
{
    m_path = path;
}
