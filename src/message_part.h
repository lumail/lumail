/*
 * message_part.h - A class for dealing with MIME parts.
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


/**
 * Only include this header one time.
 */
#pragma once


#include <string>

/**
 * This is the C++ object which represents a MIME-part from a message.
 *
 */
class CMessagePart
{
public:

    /**
     * Constructor.
     */
    CMessagePart(std::string type, std::string filename, void *content,
                 size_t content_length);

    /**
     * Destructor.
     */
    ~CMessagePart();

    /**
     * Get the content-type of the MIME-part.
     */
    std::string type();

    /**
     * Get the filename - only makes sense for "is_attachment() == true".
     */
    std::string filename();

    /**
     * Is this an attachment?
     */
    bool is_attachment();

    /**
     * Get the content.
     */
    void *content();

    /**
     * Get the length of the content.
     */
    size_t content_size();

private:

    /**
     * The content-type
     */
    std::string m_type;

    /**
     * The filename - if this is an attachment.
     */
    std::string m_filename;

    /**
     * The content of this MIME-part
     */
    void *m_content;

    /**
     * The content-length.
     */
    size_t m_content_length;
};
