/*
 * message.h - A class for handling a single message.
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


#include <memory>
#include <string>
#include <unordered_map>
#include <vector>
#include <gmime/gmime.h>

/*
 * Forward declaration of class.
 */
class CMessagePart;


/**
 *
 * This is the C++ object which represents an email message.
 *
 * The lua binding is modeled after this class structure too.
 *
 */
class CMessage
{
public:
    /**
     * Constructor.
     */
    CMessage(const std::string name, bool is_local = true);

    /**
     * Destructor.
     */
    ~CMessage();

    /**
     * Copy the message to a new maildir - which must exist.
     */
    bool copy(std::string maildir);


    /**
     * Is this message a local one?
     */
    bool is_maildir();


    /**
     * Is this message an IMAP one?
     */
    bool is_imap();


   /**
     * Get the path of this message.
     */
    std::string path();

    /**
     * Update the path to the message.
     */
    void path(std::string new_path);

    /**
     * Get the value of the given header.
     */
    std::string header(std::string name);

    /**
     * Get all headers, and their values.
     */
    std::unordered_map < std::string, std::string > headers();

    /**
     * Retrieve the current flags for this message.
     */
    std::string get_flags();

    /**
     * Set the flags for this message.
     */
    void set_flags(std::string new_flags);

    /**
     * Add a flag to a message.
     */
    bool add_flag(char c);

    /**
     * Does this message possess the given flag?
     */
    bool has_flag(char c);

    /**
     * Remove a flag from a message.
     */
    bool remove_flag(char c);

    /**
     * Is this message new?
     */
    bool is_new();

    /**
     * Mark a message as unread.
     */
    void mark_unread();

    /**
     * Mark a message as read.
     */
    void mark_read();

    /**
     * Remove this message from disk.
     */
    bool unlink();

    /**
     * Parse the message into MIME-parts, if we've not already done so.
     *
     * The parts are returned as a vector of CMessagePart objects, each
     * of which could contain nested children.
     */
    std::vector<std::shared_ptr<CMessagePart>> get_parts();

    /**
     * Add the named file as an attachment to this message.
     */
    void add_attachments(std::vector<std::string> attachments);

private:

    /**
     * Parse a MIME message and return an object suitable for operating
     * upon.
     */
    GMimeMessage * parse_message();

    /**
     * Convert a message-part from the MIME message to a CMessagePart object.
     */
    std::shared_ptr<CMessagePart> part2obj(GMimeObject *part);


private:

    /**
     * The path on-disk to the message.
     */
    std::string m_path;

    /**
     * Cached message-headers from this mail.
     */
    std::unordered_map < std::string, std::string > m_headers;

    /**
     * Cached MIME-parts to this message.
     */
    std::vector<std::shared_ptr<CMessagePart>> m_parts;

    /**
     * Is this message stored in IMAP?
     */
    bool m_imap;
};



/**
 * This is a utility-type which contains a list of messages, as
 * a vector.
 */
typedef std::vector <std::shared_ptr <CMessage > > CMessageList;
