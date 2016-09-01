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
