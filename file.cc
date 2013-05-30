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


#include <string>

#include "file.h"


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
