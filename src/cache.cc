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
    for (auto it = m_cache.begin(); it != m_cache.end(); ++it)
    {
        CacheEntry *val = it->second;

        if (val != NULL)
            delete(val);
    }

    m_cache.clear();
}


/*
 * Get the value of a cache-key.
 */
std::string CCache::get(std::string key)
{
    CacheEntry *e = m_cache[key];

    if (e != NULL)
        return (e->value);
    else
        return "";
}


/*
 * Store a value in the cache.
 */
void CCache::set(std::string key, std::string value)
{
    CacheEntry *e = m_cache[key];

    if (e != NULL)
        delete(e);

    e = new CacheEntry();
    e->value   = value;
    e->created = time(NULL);
    m_cache[ key ] = e;
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
        size_t ctime = line.find(" ");

        if (ctime != std::string::npos)
        {
            size_t kname = line.find(" ", ctime + 1);

            if (kname != std::string::npos)
            {
                std::string c_time  = line.substr(0, ctime);
                std::string k_name  = line.substr(ctime + 1, kname - ctime - 1);
                std::string k_value = line.substr(kname + 1);

                CacheEntry *e = new CacheEntry();
                e->value   = k_value;
                e->created = std::stoi(c_time);
                m_cache[ k_name ] = e;
            }
        }
    }

    fs.close();
}


/*
 * Save the map to disk.
 *
 * NOTE: We drop entries that are more than five days old.
 */
void CCache::save(std::string path)
{
    std::fstream fs;
    fs.open(path,  std::fstream::out);

    /*
     * Get the current time, and work out five days ago.
     */
    time_t now = time(NULL);
    now -= (60 * 60 * 24 * 5);

    /*
     * Iterate over our cache-map.
     */
    for (auto it = m_cache.begin(); it != m_cache.end(); ++it)
    {
        std::string key = it->first;
        CacheEntry *val = it->second;

        /*
         * If the key and value are non-empty AND the cache-key was
         * set then the past week then persist it.
         */
        if (!key.empty() && (val != NULL) && (val->created > now))
            fs << val->created << " " << key << " " << val->value << std::endl;
    }

    fs.close();
}
