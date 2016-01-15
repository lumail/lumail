/*
 * maildir.h - Wrapper for a Maildir.
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


/*
 * Only include this header one time.
 */
#pragma once


#include "message.h"




/**
 * This is the C++ implementation of the maildir class, which allows
 * simple operations to be carried out against Maildir folders stored
 * upon the local filesystem.
 *
 */
class CMaildir
{
public:

    /**
     * Constructor.
     */
    CMaildir(const std::string name);


    /**
     * Destructor.
     */
    ~CMaildir();


    /**
     * Return the path we represent.
     */
    std::string path();


    /**
     * The number of new messages for this maildir.
     */
    int unread_messages();


    /**
     * The total number of messages for this maildir.
     */
    int total_messages();


    /**
     * Get all of the messages in this maildir.
     */
    CMessageList getMessages();


    /**
     * Save the given message in this maildir.
     */
    bool saveMessage(std::shared_ptr <CMessage > msg);

private:

    /**
     * The path we represent.
     */
    std::string m_path;

    /**
     * The date/time this maildir was last updated.  Used to maintain a
     * cache that can be expired/tested easily.
     */
    time_t m_modified;

    /**
     * A cached count of the unread messages in this maildir.
     */
    int m_unread;

    /**
     * A cached count of messages in this maildir.
     */
    int m_total;

    /**
     * Return the last modified time for this Maildir.
     * Used to determine if we need to update our cache.
     */
    time_t last_modified();

    /**
     * Update the cached total/unread message counts.
     */
    void update_cache();

    /**
     * Generate a filename for saving a message into.
     */
    std::string generate_filename(bool is_new);

};


/**
 * This is a utility-type which contains a list of maildirs, as
 * a vector.
 */
typedef std::vector<std::shared_ptr<CMaildir> > CMaildirList;
