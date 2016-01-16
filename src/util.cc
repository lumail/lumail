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


#include "util.h"



/*
 * URL-encode a string.
 */
std::string urlencode(const std::string &s)
{
    static const char lookup[] = "0123456789abcdef";
    std::stringstream e;

    for (int i = 0, ix = s.length(); i < ix; i++)
    {
        const char& c = s[i];

        if ((48 <= c && c <= 57) || //0-9
                (65 <= c && c <= 90) ||//abc...xyz
                (97 <= c && c <= 122) || //ABC...XYZ
                (c == '-' || c == '_' || c == '.' || c == '~')
           )
        {
            e << c;
        }
        else
        {
            e << '%';
            e << lookup[(c & 0xF0)>>4 ];
            e << lookup[(c & 0x0F) ];
        }
    }

    return e.str();
};


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
};


/*
 * execute a command and return the output as a vector of lines.
 */
std::vector<std::string> shell_execute(std::string cmd)
{
    FILE*           fp;

    std::vector<std::string> result;

    if ((fp = popen(cmd.c_str(), "r")) == NULL)
        return (result);


    while (!feof(fp))
    {
        char buf[ 1024 ] = {0};

        if (fgets(buf, sizeof(buf), fp) > 0)
        {
            result.push_back(std::string(buf));
        }
    }

    pclose(fp);

    return (result);
}
