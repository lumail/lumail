/*
 * directory.h - Simple directory handling functions.
 *
 * This file is part of lumail - http://lumail.org/
 *
 * Copyright (c) 2015 by Steve Kemp.  All rights reserved.
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


/**
 * Only include this header one time.
 */
#pragma once


#include <vector>
#include <string>

/**
 * A collection of directory utility primitives.
 *
 * Each of these members are static, because these are really just utility
 * functions which aren't used so much.  Some of these functions are wrapped
 * to Lua, via the implementation in `directory_lua.cc`.
 */
class CDirectory
{

public:

    /**
     * Does the directory exist?
     */
    static bool exists(std::string path);

    /**
     * Return a sorted list of files beneath the directory.
     */
    static std::vector < std::string > entries(std::string prefix);

};
