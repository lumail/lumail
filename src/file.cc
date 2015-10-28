/**
 * $FILENAME - $TITLE
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
#include <iostream>
#include <fstream>
#include <sstream>
#include <algorithm>
#include <iterator>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <dirent.h>
#include <unistd.h>
#include <assert.h>


#include "file.h"
#include "maildir.h"


#ifndef FILE_READ_BUFFER
#define FILE_READ_BUFFER 16384
#endif



/**
 * Test if a file exists.
 */
bool CFile::exists(std::string path)
{
    struct stat sb;

    if ((stat(path.c_str(), &sb) == 0))
        return true;
    else
        return false;
}


/**
 * Is the given path a directory?
 */
bool CFile::is_directory(std::string path)
{
    struct stat sb;

    if (stat(path.c_str(), &sb) < 0)
        return false;

    return (S_ISDIR(sb.st_mode));
}


/**
 * Get the basename of a file.
 */
std::string CFile::basename(std::string path)
{
    size_t
    offset = path.find_last_of("/");

    if (offset != std::string::npos)
        return (path.substr(offset + 1));
    else
        return (path);
}


/**
 * Move a file.
 */
bool CFile::move(std::string src, std::string dst)
{
    int ret = rename(src.c_str(), dst.c_str());

    assert(CFile::exists(dst));
    assert(!CFile::exists(src));

    return (ret == 0);
}


/**
 * Return a sorted list of maildirs beneath the given prefix.
 */
std::vector < std::string > CFile::get_all_maildirs(std::string prefix)
{
    std::vector < std::string > result;

    dirent *de;
    DIR *dp;

    dp = opendir(prefix.c_str());

    if (dp)
    {
        if (CMaildir::is_maildir(prefix))
            result.push_back(prefix);

        while (true)
        {
            de = readdir(dp);

            if (de == NULL)
                break;

            if ((strcmp(de->d_name, ".") != 0) &&
                    (strcmp(de->d_name, "..") != 0) &&
                    ((de->d_type == DT_UNKNOWN) || (de->d_type == DT_DIR)))
            {
                std::string subdir_name = std::string(de->d_name);
                std::string subdir_path =
                    std::string(prefix + "/" + subdir_name);

                if (CMaildir::is_maildir(subdir_path))
                    result.push_back(subdir_path);
                else
                {
                    if (subdir_name != "." && subdir_name != "..")
                    {
                        DIR *sdp = opendir(subdir_path.c_str());

                        if (sdp)
                        {
                            closedir(sdp);
                            std::vector < std::string > sub_maildirs;
                            sub_maildirs = CFile::get_all_maildirs(subdir_path);

                            for (std::string sub_maildir : sub_maildirs)
                            {
                                result.push_back(sub_maildir);
                            }
                        }
                    }
                }
            }
        }

        closedir(dp);
        std::sort(result.begin(), result.end());
    }

    return result;
}
