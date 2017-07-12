/*
 * file.h - Simple file-handling functions.
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


#include <vector>
#include <string>


/**
 *
 * A collection of file utility primitives.
 *
 * Each of these members are static, because these are really just utility
 * functions which aren't used so much.  Some of these functions are wrapped
 * to Lua, via the implementation in `file_lua.cc`.
 *
 */
class CFile
{

public:

    /**
       * Does the given path exist?
       */
    static bool exists(std::string path);

    /**
     * Is the given path a directory?
     */
    static bool is_directory(std::string path);

    /**
     * Is the given path a maildir?
     */
    static bool is_maildir(std::string path);

    /**
     * Delete the given path.
     */
    static bool delete_file(std::string path);

    /**
     * Expand a given path, converting "$HOME" and "~/" appropriately
     */
    static std::string expand_path(std::string path);

    /**
     * Get the basename of the given file.
     */
    static std::string basename(std::string path);

    /**
     * Copy the given file.
     */
    static bool copy(std::string src, std::string dst);

    /**
     * Move the given file.
     */
    static bool move(std::string src, std::string dst);

    /**
     * Get the size of the given file, in bytes.  -1 on error.
     */
    static int size(std::string path);


    /**
     * Return a sorted list of maildirs beneath the given prefix.
     */
    static std::vector < std::string > get_all_maildirs(std::string prefix);

};
