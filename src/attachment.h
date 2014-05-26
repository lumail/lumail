/**
 * attachment.h - A class for storing message-attachment data.
 *
 * This file is part of lumail: http://lumail.org/
 *
 * Copyright (c) 2014 by Steve Kemp.  All rights reserved.
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

#ifndef _attachment_h
#define _attachment_h 1

#include <string.h>

#include "utfstring.h"
#include <unordered_map>


/**
 * A small class for holding and retrieving the attachments associated
 * with a particular message.
 *
 * All the code here is inline, because the class is nothing more
 * than a simple wrapper object with no particular logic.
 *
 */
class CAttachment
{
public:

    /**
     * Constructor.
     */
    CAttachment(UTFString name, void * body, size_t sz )
    {
        /**
         * Save the name away.
         */
        m_name = name;

        /**
         * Save the data away
         */
        m_data = (void *)malloc( sz );
        assert(m_data);

        memcpy( m_data, body, sz );
        m_size = sz;
    };


    /**
     * Destructor.
     */
    ~CAttachment()
    {
        if ( m_data != NULL )
        {
            free( m_data );
            m_data = NULL;
        }
    }


    /**
     * Return the (file)name of the attachment.
     */
    UTFString name() { return m_name; }


    /**
     * Return the body of the attachment.
     */
    void *body() { return m_data; }


    /**
     * Return the size of the attachment.
     */
    size_t size() { return m_size; }

private:
    /**
     * Stored objects.
     */
    UTFString m_name;
    void    * m_data;
    size_t    m_size;
};



#endif /* _attachment_h */
