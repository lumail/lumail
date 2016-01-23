/*
 * util.h - Misc utility functions.
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
#include <sstream>


/**
 * A filter which is used by `std::erase` to remove duplicate slash-characters.
 *
 * Usage looks like this:
 *
 * <pre>
 * std::string in = "this//has/too/many//slashes.txt";
 * in.erase(std::unique(in.begin(), in.end(), both_slashes()), in.end());
 *
 * // in now contains 'this/has/too/many/slashes.txt'.
 * </pre>
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



/**
 * Split a string into a vector of strings on the given character.
 */
std::vector<std::string> split(const std::string &text, char sep);


/**
 * Escape a string such that it can be used for a filename.
 *
 * For example "`foo/bar`" would become "`foo_bar`", and
 * `imaps://example.com/` would become "`imaps:__example.com_`"
 */
std::string escape_filename(std::string path);
