/*
 * cache.h - Simple in-RAM cache.
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


#pragma once


#include <string>
#include <unordered_map>


/**
 * A cached entry.
 *
 * This structure contains a cache-value, along with the time that
 * it was inserted into the cache.
 */
class CacheEntry
{
public:
    std::string value;
    time_t      created;
};

/**
 *
 * A simple in-RAM cache.
 *
 */
class CCache
{

public:

    /**
     * Constructor
     */
    CCache();

    /**
     * Destructor
     */
    ~CCache();


public:

    /**
     * Empty the cache, removing all keys/values
     */
    void empty();

    /**
     * Get the value of a cache-key.
     */
    std::string get(std::string key);

    /**
     * Load the map from disk.
     */
    void load(std::string path);

    /**
     * Save the map to disk.
     */
    void save(std::string path);

    /**
     * Store a value in the cache.
     */
    void set(std::string key, std::string value);

private:

    /**
     * The actual map which stores our cached data.
     */
    std::unordered_map <std::string, CacheEntry * >m_cache;

};
