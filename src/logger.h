/*
 * logger.h - Simple singleton for logging messages.
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
 * This is a simple Singleton class to allow our code to write log
 * messages.
 *
 * It can be used by the C++ code, or via the Lua wrapper.
 */
class CLogger : public Singleton<CLogger>
{

public:

    /**
     * Log a message.
     */
    void log(const char *level , const char *fmt,  ...);

    /**
     * Change the log-level.
     */
    void set_level(std::string level);

    /**
     * Change the log-file.
     */
    void set_path(std::string path);

public:

    /**
     * Constructor.
     */
    CLogger();

private:

    /**
     * The current log-level.
     */
    std::string m_level;

    /**
     * The log-file
     */
    std::string m_path;
};
