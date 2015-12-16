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
 * A singleton class holding an instance of `libmagic`, which is
 * used for identifying the MIME-type of files.
 *
 * We use this specifically to identify the MIME-type of files which
 * are attached to outgoing messages.
 *
 * Usage is as simple as:
 *
 * <pre>
 *  CMime  *instance = CMime::Instance();
 *  std::string type = instance->type( "/etc/passwd" );
 * </pre>
 *
 * Failure to identify the MIME-type of the specified file will
 * result in the default value of `application/octet-stream` being
 * returned.
 *
 */
class CMime : public Singleton<CMime>
{
public:
    /**
     * Constructor.  Open up a handle to `libmagic`.
     */
    CMime();

    /**
     * Destructor.  Close the `libmagic` handle.
     */
    ~CMime();

public:

    /**
     * Discover the MIME-type of the specified file.
     *
     * If this cannot be determined return the default value which was
     * specified.
     */
    std::string type(std::string file, std::string def_type = "application/octet-stream" );

private:

    /**
     * The handle provided by `libmagic`.
     */
    magic_t m_mime;

};
