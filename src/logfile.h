/*
 * logfile.h - Simple singleton for logging output messages.
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
class CLogfile : public Singleton<CLogfile>
{

public:

    /**
     * Append a new string to the logfile.
     */
    void append(std::string entry);

    /**
     * Set the file to log to.
     */
    void set_file(std::string path);

public:

    /**
     * Constructor.
     */
    CLogfile();

private:

    /**
     * The file to write to, may be unset.
     */
    std::string m_filename;

};
