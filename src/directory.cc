/*
 * directory.cc - Simple directory handling functions.
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

#include <algorithm>
#include <sys/stat.h>
#include <dirent.h>

#include "directory.h"


/**
 * Does the directory exist?
 */
bool CDirectory::exists(std::string path)
{
    struct stat sb;

    if ((stat(path.c_str(), &sb) == 0))
        return true;
    else
        return false;
}


/**
 * Return a sorted list of files beneath the directory.
 */
std::vector < std::string > CDirectory::entries(std::string prefix)
{
    std::vector < std::string > result;

    /**
     * Strip "/" from the end of the string, if present.
     */
    if (prefix.rfind("/") == (prefix.size() - 1))
        prefix = prefix.substr(0, prefix.size() - 1);


    dirent *de;
    DIR *dp;

    if ((dp = opendir(prefix.c_str())) != NULL)
    {
        result.push_back(prefix);

        while ((de = readdir(dp)) != NULL)
        {
            result.push_back(prefix + "/" + de->d_name);
        }
    }

    std::sort(result.begin(), result.end());

    return result;
}
