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


#include <vector>
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
     * Get the files in the given directory.
     *
     * NOTE: Directories are excluded.
     */
    static std::vector<std::string> files_in_directory(std::string path);


    /**
     * Delete a file.
     */
    static bool delete_file(std::string path);


    /**
     * Get the basename of a file.
     */
    static std::string basename( std::string path );


    /**
     * Copy a file.
     */
    static void copy( std::string src, std::string dest );


    /**
     * Edit a file, using the users preferred editor.
     */
    static int edit( std::string filename );


    /**
     * Move a file.
     */
    static bool move( std::string src, std::string dest );


    /**
     * Send the contents of a file to the given command, via popen.
     */
    static bool file_to_pipe( std::string src, std::string cmd );


    /**
     * Return a sorted list of maildirs beneath the given prefix.
     */
    static std::vector<std::string> get_all_maildirs(std::string prefix);

};

#endif /* _file_h_ */
