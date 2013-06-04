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

#include <iostream>
#include <fstream>
#include <algorithm>
#include <iterator>
#include <cstdlib>
#include <string>
#include <sys/stat.h>
#include <sys/types.h>


#include "debug.h"
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
 * Is the given file executable?
 */
bool CFile::executable( std::string path )
{
    struct stat sb;

    /**
     * Fail to stat?  Then not executable.
     */
    if (stat(path.c_str(), &sb) < 0)
        return false;

    /**
     * If directory then not executable.
     */
    if (S_ISDIR(sb.st_mode))
        return false;

    /**
     * Otherwise check the mode-bits
     */
    if ((sb.st_mode & S_IEXEC) != 0)
        return true;
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

#ifdef LUMAIL_DEBUG
    std::string dm = "CFile::copy(";
    dm += src ;
    dm += ",";
    dm += dst;
    dm += ");";
    DEBUG_LOG( dm );
#endif

    std::ifstream isrc(src, std::ios::binary);
    std::ofstream odst(dst, std::ios::binary);

    std::istreambuf_iterator<char> begin_source(isrc);
    std::istreambuf_iterator<char> end_source;
    std::ostreambuf_iterator<char> begin_dest(odst);

    std::copy(begin_source, end_source, begin_dest);

    isrc.close();
    odst.close();
}

/**
 * Move a file.
 */
bool CFile::move( std::string src, std::string dst )
{
#ifdef LUMAIL_DEBUG
    std::string dm = "CFile::move(";
    dm += src ;
    dm += ",";
    dm += dst;
    dm += ");";
    DEBUG_LOG( dm );
#endif

    return( rename( src.c_str(), dst.c_str() ) == 0 );
}
