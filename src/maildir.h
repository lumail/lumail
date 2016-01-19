/*
 * maildir.h - Wrapper for a collection of messages.
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
 * simple operations to be carried out against collections of folders.
 *
 * Traditionally this class was solely responsible for the handling
 * of local Maildir-folders, however it has been expanding in scope
 * such that a CMaildir object can now represent either a local Maildir
 * directory _or_ a remote IMAP directory.
 *
 * To differentiate the two a constructor flag is used, and the two
 * member-methods `is_maildir` and `is_imap` are exported to allow
 * callers to tell the difference.
 *
 */
class CMaildir
{
public:

    /**
     * Constructor.  If the `is_local` flag is true then the name
     * points to the fully-qualified path, on disk.  Otherwise the
     * name will be a string such as "Sent", "INBOX", or similar
     * which lives upon a remote IMAP-server.
     */
    CMaildir(const std::string name, bool is_local = true);


    /**
     * Destructor.
     */
    ~CMaildir();


    /**
     * Return the path we represent, which might be a local
     * maildir-location, or a remote IMAP path.
     *
     * Use "is_imap" or "is_maildir" to tell the difference.
     */
    std::string path();


    /**
     * Is this maildir a local one?
     */
    bool is_maildir();


    /**
     * Is this maildir an IMAP path?
     */
    bool is_imap();


    /**
     * Retrieve the number of new messages for this maildir.
     *
     * In the case of a local Maildir this is discovered by opening
     * the directory and counting the messages.  For IMAP servers we
     * rely upon this being passed to us at construction-time, and
     * updated when state-changes via `set_unread()`.
     */
    int unread_messages();


    /**
     * Set the number of unread messages in this maildir.
     *
     * This is called when the object is created __if__ the object
     * is a remote IMAP folder.   If that is the case then the notion
     * of updating never applies.
     */
    void set_unread(int n)
    {
        m_unread = n;
    };

    /**
     * Retrieve total number of messages for this maildir.
     *
     * In the case of a local Maildir this is discovered by opening
     * the directory and counting the messages.  For IMAP servers we
     * rely upon this being passed to us at construction-time, and
     * updated when state-changes via `set_unread()`.
     */
    int total_messages();


    /**
     * Set the number of total messages in this maildir.
     *
     * This is called when the object is created __if__ the object
     * is a remote IMAP folder.   If that is the case then the notion
     * of updating never applies.
     */
    void set_total(int n)
    {
        m_total = n;
    };


   /**
     * Get all of the messages in this maildir.
     */
    CMessageList getMessages();


    /**
     * Save the given message in this maildir.
     *
     * For a local message this is done directly, for a remote one
     * the perl-helper `perl.d/save-message` is invoked.
     */
    bool saveMessage(std::shared_ptr <CMessage > msg);


    /**
     * Bump the modification-time of this maildir artificially.
     *
     * **NOTE**: This is used solely for IMAP-based messages, and
     * updates the fake time that `last_modified()` returns.
     */
    void bump_mtime();


    /**
     * Return the last modified time for this Maildir, which is
     * used to determine if we need to update our cache.
     *
     * **NOTE**: This result is faked for IMAP-folders, via `bump_mtime()`.
     */
    time_t last_modified();

private:

    /**
     * The path we represent.
     */
    std::string m_path;

    /**
     * Are we an IMAP maildir?
     */
    bool m_imap;

    /**
     * The date/time this maildir was last updated.  Used to maintain a
     * cache that can be expired/tested easily.
     *
     * **NOTE**: This does not apply to IMAP folders.
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
     * Update the cached total/unread message counts.
     *
     * **NOTE**: This is a NOP for IMAP folders.
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
