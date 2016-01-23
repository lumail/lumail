/*
 * util.cc - Misc. utility functions.
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


#include <cstdlib>
#include <memory>
#include <stdlib.h>
#include <string.h>

#include "util.h"



/*
 * Split a string into a vector of strings on the given character.
 */
std::vector<std::string> split(const std::string &text, char sep)
{
    std::vector<std::string> tokens;
    std::size_t start = 0, end = 0;

    while ((end = text.find(sep, start)) != std::string::npos)
    {
        tokens.push_back(text.substr(start, end - start));
        start = end + 1;
    }

    tokens.push_back(text.substr(start));
    return tokens;
}


/*
 * Escape a string such that it can be used for a filename.
 *
 * For example "`foo/bar`" would become "`foo_bar`", and
 * `imaps://example.com/` would become "`imaps:__example.com_`"
 */
std::string escape_filename(std::string path)
{
    std::transform(path.begin(), path.end(), path.begin(), [](char ch)
    {
        return (ch == '/' || ch == '\\') ? '_' : ch;
    });

    return (path);
}
