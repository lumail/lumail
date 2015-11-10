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


#include <string>
#include <vector>
#include <memory>


/*
 * Forward declaration of class.
 */
class CMessage;

/*
 * Type of a list of messages.
 */
typedef std::vector <std::shared_ptr <CMessage > > CMessageList;


/*
 * Maildir object.
 *
 * This is the C++ implementation of the maildir class.
 *
 */
class CMaildir
{
public:

    CMaildir(const std::string name);
    ~CMaildir();

    /*
     * Return the path we represent.
     */
    std::string path();

    /*
     * The number of new messages for this maildir.
     */
    int unread_messages();


    /*
     * The total number of messages for this maildir.
     */
    int total_messages();


    /*
     * Get all messages in this maildir.
     */
    CMessageList getMessages();


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


    /*
     * Return the last modified time for this Maildir.
     * Used to determine if we need to update our cache.
     */
    time_t last_modified();


    /*
     * Update the cached total/unread message counts.
     */
    void update_cache();

};


/*
 * Helper-type for working with message-lists.
 */
typedef std::vector<std::shared_ptr<CMaildir> > CMaildirList;
