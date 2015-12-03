/*
 * mime.h - Wrapper for libmagic
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

#include <magic.h>
#include <string>

#include "singleton.h"

/**
 * A singleton class holding an instance of libmagic, which is
 * used for identifying the MIME-type of files.
 *
 */
class CMime : public Singleton<CMime>
{
public:
    CMime();
    ~CMime();

public:

    /**
     * Get the MIME type of the given file.
     */
    std::string type(std::string file);

private:

    /**
     * The handle to the mime-library.
     */
    magic_t m_mime;

};
