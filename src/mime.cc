/*
 * mime.cc - Wrapper for libmagic
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


#include "mime.h"



/*
 * Constructor - This is private as this class is a singleton.
 */
CMime::CMime()
{
    m_mime = magic_open(MAGIC_MIME_TYPE);

    if (m_mime == NULL)
    {
        throw "Failed to init libmagic";
    }

    if (magic_load(m_mime, NULL) != 0)
    {
        throw "Failed to load magic database";
    }
}


/*
 * Destructor.
 */
CMime::~CMime()
{
    magic_close(m_mime);
}

/*
 * Discover the MIME-type of the specified file.
 *
 * If this cannot be determined return the default value which will be
 * `application/octet-stream`.
 */
std::string CMime::type(std::string file, std::string def_type)
{
    const char *info = magic_file(m_mime, file.c_str());

    if (info)
        return (info);
    else
        return (def_type);
}
