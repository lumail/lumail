/*
 * cache.cc - Simple in-RAM cache.
 *
 * This file is part of lumail - http://lumail.org/
 *
 * Copyright (c) 2016 by Steve Kemp.  All rights reserved.
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


#include <fstream>

#include "cache.h"


/*
 * Constructor.
 */
CCache::CCache()
{
}

/*
 * Destructor
 */
CCache::~CCache()
{
}


/*
 * Empty the cache, removing all keys/values
 */
void CCache::empty()
{
    m_cache.clear();
}


/*
 * Get the value of a cache-key.
 */
std::string CCache::get(std::string key)
{
    return (m_cache[key]);
}


/*
 * Store a value in the cache.
 */
void CCache::set(std::string key, std::string value)
{
    m_cache[ key ] = value;
}


/*
 * Load the map from disk.
 */
void CCache::load(std::string path)
{
    /*
     * Empty any existing members.
     */
    empty();

    /*
     * Open the file.
     */
    std::fstream fs;
    fs.open(path,  std::fstream::in);

    /*
     * Process each line.
     */
    for (std::string line; getline(fs, line);)
    {
        size_t offset = line.find(" ");

        if (offset != std::string::npos)
        {
            std::string key = line.substr(0, offset);
            std::string val = line.substr(offset + 1);

            m_cache[key] = val;
        }
    }

    fs.close();
}


/*
 * Save the map to disk.
 */
void CCache::save(std::string path)
{
    std::fstream fs;
    fs.open(path,  std::fstream::out);

    for (auto it = m_cache.begin(); it != m_cache.end(); ++it)
    {
        std::string key = it->first;
        std::string val = it->second;

        if ((!key.empty()) && (!val.empty()))
        {
            fs << key << " " << val << std::endl;
        }
    }

    fs.close();
}
