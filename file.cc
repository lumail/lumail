/**
 * file.cc - Simple file primitives.
 *
 * This file is part of lumail: http://lumail.org/
 *
 * Copyright (c) 2013 by Steve Kemp.  All rights reserved.
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
#include <string>
#include <sys/stat.h>
#include <sys/types.h>

#include "file.h"


/**
 * Test if a file exists.
 */
bool CFile::exists( std::string path )
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
      return 0;

    return (S_ISDIR(sb.st_mode));
}


/**
 * Copy a file.
 */
void CFile::copy( std::string src, std::string dst )
{
    std::string cmd = "/bin/cp ";
    cmd += src;
    cmd += " ";
    cmd += dst;

    system( cmd.c_str() );
}


/**
 * Move a file.
 */
void CFile::move( std::string src, std::string dst )
{
    std::string cmd = "/bin/mv ";
    cmd += src;
    cmd += " ";
    cmd += dst;

    system( cmd.c_str() );
}
