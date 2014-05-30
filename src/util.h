/**
 * util.h - C++ utility functions.
 *
 * This file is part of lumail: http://lumail.org/
 *
 * Copyright (c) 2014 by Steve Kemp.  All rights reserved.
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

#include "utfstring.h"


/**
 * A static class of utility functions.
 */
class CUtil
{
public:

    /**
     * Convert a string to an array, by tokenizing on a given deliminator.
     */
    static std::vector<UTFString> &split(const std::string &s, char delim, std::vector<UTFString> &elems)
    {
        std::stringstream ss(s);
        std::string item;
        while (std::getline(ss, item, delim))
        {
            elems.push_back(item);
        }
        return elems;
    };


    /**
     * Convert a string to an array, by tokenizing on a given deliminator.
     */
    static std::vector<UTFString> split(const UTFString &s, char delim)
    {
        std::vector<UTFString> elems;
        split(s, delim, elems);
        return elems;
    };

};
