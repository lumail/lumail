/**
 * file.h - Simple file primitives.
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


#ifndef _file_h_
#define _file_h_ 1



#include <string>


/**
 * A collection of file utility primitives.
 */
class CFile
{

public:

    /**
     * Does a file exist?
     */
    static bool exists( std::string path );


    /**
     * Is the given file executable?
     */
    static bool executable( std::string path );


    /**
     * Is the given path a directory?
     */
    static bool is_directory(std::string path);


    /**
     * Copy a file.
     */
    static void copy( std::string src, std::string dest );


    /**
     * Move a file.
     */
    static bool move( std::string src, std::string dest );

};

#endif /* _file_h_ */
