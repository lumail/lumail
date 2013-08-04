/**
 * message.h - A class for working with a single message.
 *
 * This file is part of lumail: http://lumail.org/
 *
 * Copyright (c) 2013 by Steve Kemp.  All rights reserved.
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

#ifndef _message_h
#define _message_h 1


#include <string>
#include <stdint.h>
#include <mimetic/mimetic.h>


/**
 * A class for working with a single message.
 *
 * The constructor will be passed a reference to a filename, which is assumed to be file
 * beneath a Maildir folder.
 *
 * Using the mimetic library we'll parse the message and make various fields available.
 *
 */
class CMessage
{
private:
    time_t m_time_cache;

public:

    /**
     * Used for formatting dates.
     */
    enum TDate { EYEAR, EDAY, EMONTH, EMON, EFULL };


    /**
     * Constructor
     */
    CMessage(std::string filename);

    /**
     * Destructor.
     */
    ~CMessage();

    /**
     * Get the path to the message, on-disk.
     */
    std::string path();

    /**
     * Update the path to the message, on-disk.
     */
    void path( std::string new_path );

    /**
     * Format the message for display in the header - via the lua format string.
     */
    std::string format( std::string fmt = "");

    /**
     * Get the flags for this message.
     */
    std::string flags();

    /**
     * Add a flag to a message.
     */
    void add_flag( char c );

    /**
     * Does this message possess the given flag?
     */
    bool has_flag( char c );

    /**
     * Remove a flag from a message.
     */
    void remove_flag( char c );

    /**
     * Does this message match the given filter?
     */
    bool matches_filter( std::string *filter );

    /**
     * Is this message new?
     */
    bool is_new();

    /**
     * Mark a message as new.
     */
    bool mark_new();

    /**
     * Mark a message as read.
     */
    bool mark_read();

    /**
     * Get the message last modified time (cached).
     */
    const time_t mtime();

    /**
     * get a header from the message.
     */
    std::string header( std::string name);

    /**
     * Get the sender of the message.
     */
    std::string from();

    /**
     * Get the date of the message.
     */
    std::string date(TDate fmt = EFULL);

    /**
     * Get the recipient of the message.
     */
    std::string to();

    /**
     * Get the subject of the message.
     */
    std::string subject();

    /**
     * Get the body of the message, as a vector of lines.
     */
    std::vector<std::string> body();

    /**
     * Get the names of attachments to this message.
     */
    std::vector<std::string> attachments();

    /**
     * Save the given attachment.
     */
    bool save_attachment( int offset, std::string output_path );

    /**
     * This is solely used for sorting by message-headers
     */
    time_t get_date_field();

private:

    /**
     * Parse the message, if that hasn't been done.
     * Uses "on_message_parse" if that is defined.
     */
    void message_parse();

    /**
     * Attempt to find a MIME-part inside our message of the given type.
     */
    std::string getMimePart(mimetic::MimeEntity* pMe, std::string mtype );

    /**
     * Decode a header-field.
     */
    std::string decode_field( std::string str );

    /**
     * The file we represent.
     */
    std::string m_path;

    /**
     * MIME Entity object for this message.
     */
    mimetic::MimeEntity *m_me;

    /**
     * Cached time/date object.
     */
    time_t m_date;

};

#endif /* _message_h */
