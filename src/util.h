/*
 * util.h - Misc. STL-utility functions.
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


#pragma once

#include <algorithm>


/**
 * A filter which is used by std::erase to remove duplicate slash-characters.
 */
struct both_slashes
{
    /**
     * This implements the filter process, returning true if both
     * characters "a" and "b" are "/".
     */
    bool operator()(char a, char b) const
    {
        return a == '/' && b == '/';
    }
};
