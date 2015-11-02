/**
 * $FILENAME - $TITLE
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


#include <algorithm>
#include <cstdlib>
#include <fcntl.h>
#include <fstream>
#include <iostream>
#include <string.h>
#include <string>
#include <sstream>
#include <sys/types.h>
#include <unistd.h>


#include <gmime/gmime.h>

#include "file.h"
#include "message.h"
#include "message_part.h"



/**
 * Constructor.
 */
CMessage::CMessage(const std::string name)
{
    m_path = name;
}


/**
 * Return the path to this message.
 */
std::string CMessage::path()
{
    return (m_path);
}


/**
 * Update the path to the message.
 */
void CMessage::path(std::string new_path)
{
    m_path = new_path;
}


/**
 * Return the value of a given header.
 */
std::string CMessage::header(std::string name)
{
    std::unordered_map < std::string, std::string > h = headers();

    /**
     * Lower-case the header we were given.
     */
    std::transform(name.begin(), name.end(), name.begin(), tolower);

    /**
     * Lookup the value.
     */
    return (h[name]);
}


/**
 * Return all header-names, and their values.
 */
std::unordered_map < std::string, std::string > CMessage::headers()
{
    std::unordered_map < std::string, std::string > m_header_values;

    GMimeMessage *
    m_message;
    GMimeParser *
    parser;
    GMimeStream *
    stream;
    int
    fd;

    if ((fd = open(m_path.c_str(), O_RDONLY, 0)) == -1)
        throw "Opening the message failed";

    stream = g_mime_stream_fs_new(fd);

    parser = g_mime_parser_new_with_stream(stream);
    g_object_unref(stream);

    m_message = g_mime_parser_construct_message(parser);
    g_object_unref(parser);


    const char *
    name;
    const char *
    value;

    /**
     * Prepare to iterate.
     */
    GMimeHeaderList *
    ls = GMIME_OBJECT(m_message)->headers;
    GMimeHeaderIter *
    iter = g_mime_header_iter_new();

    if (g_mime_header_list_get_iter(ls, iter)
            && g_mime_header_iter_first(iter))
    {
        while (g_mime_header_iter_is_valid(iter))
        {
            /**
             * Get the name + value.
             */
            name = g_mime_header_iter_get_name(iter);
            value = g_mime_header_iter_get_value(iter);

            /**
             * Downcase the name.
             */
            std::string nm(name);
            std::transform(nm.begin(), nm.end(), nm.begin(), tolower);


            /**
             * Decode the value.
             */
            char *
            decoded = g_mime_utils_header_decode_text(value);
            m_header_values[nm] = decoded;


            if (!g_mime_header_iter_next(iter))
                break;
        }
    }

    g_mime_header_iter_free(iter);


    g_object_unref(m_message);

    return (m_header_values);
}


/**
 * Destructor: If we've parsed any MIME-parts then free them here.
 */
CMessage::~CMessage()
{
    if (m_parts.size() > 0)
        m_parts.clear();
}


/**
 * Retrieve the current flags for this message.
 */
std::string CMessage::get_flags()
{
    std::string flags = "";
    std::string pth = path();

    if (pth.empty())
        return (flags);

    size_t
    offset = pth.find(":2,");

    if (offset != std::string::npos)
        flags = pth.substr(offset + 3);

    /**
     * Sleazy Hack.
     */
    if (pth.find("/new/") != std::string::npos)
        flags += "N";


    /**
     * Sort the flags, and remove duplicates
     */
    std::sort(flags.begin(), flags.end());
    flags.erase(std::unique(flags.begin(), flags.end()), flags.end());

    return flags;
}


/**
 * Set the flags for this message.
 */
void CMessage::set_flags(std::string new_flags)
{
    /**
     * Sort the flags.
     */
    std::string flags = new_flags;
    std::sort(flags.begin(), flags.end());
    flags.erase(std::unique(flags.begin(), flags.end()), flags.end());

    /**
     * Get the current ending position.
     */
    std::string cur_path = path();
    std::string dst_path = cur_path;

    size_t offset = std::string::npos;

    if ((offset = cur_path.find(":2,")) != std::string::npos)
    {
        dst_path = cur_path.substr(0, offset);
        dst_path += ":2,";
        dst_path += flags;
    }
    else
    {
        dst_path = cur_path;
        dst_path += ":2,";
        dst_path += flags;
    }

    if (cur_path != dst_path)
    {
        CFile::move(cur_path, dst_path);
        path(dst_path);
    }
}


/**
 * Add a flag to a message.
 *
 * Return true if the flag was added, false if already present.
 */
bool CMessage::add_flag(char c)
{
    std::string flags = get_flags();

    size_t offset = std::string::npos;

    /**
     * If the flag was missing, add it.
     */
    if ((offset = flags.find(c)) == std::string::npos)
    {
        flags += c;
        set_flags(flags);
        return true;
    }
    else
        return false;
}


/**
 * Does this message possess the given flag?
 */
bool CMessage::has_flag(char c)
{
    /**
     * Flags are upper-case.
     */
    c = toupper(c);

    if (get_flags().find(c) != std::string::npos)
        return true;
    else
        return false;
}

/**
 * Remove a flag from a message.
 *
 * Return true if the flag was removed, false if it wasn't present.
 */
bool CMessage::remove_flag(char c)
{
    c = toupper(c);

    std::string current = get_flags();

    /**
     * If the flag is not present, return.
     */
    if (current.find(c) == std::string::npos)
        return false;

    /**
     * Remove the flag.
     */
    std::string::size_type k = 0;

    while ((k = current.find(c, k)) != current.npos)
        current.erase(k, 1);

    set_flags(current);

    return true;
}

/**
 * Is this message new?
 */
bool CMessage::is_new()
{
    /**
     * A message is new if:
     *
     * It has the flag "N".
     * It does not have the flag "S".
     */
    if ((has_flag('N')) || (!has_flag('S')))
        return true;

    return false;
}


/**
 * Mark a message as unread.
 */
void CMessage::mark_unread()
{
    if (has_flag('S'))
        remove_flag('S');

}

/**
 * Mark a message as read.
 */
void CMessage::mark_read()
{
    /*
     * Get the current path, and build a new one.
     */
    std::string c_path = path();
    std::string n_path = "";

    size_t offset = std::string::npos;

    /**
     * If we find /new/ in the path then rename to be /cur/
     */
    if ((offset = c_path.find("/new/")) != std::string::npos)
    {
        /**
         * Path component before /new/ + after it.
         */
        std::string before = c_path.substr(0, offset);
        std::string after  = c_path.substr(offset + strlen("/new/"));

        n_path = before + "/cur/" + after;

        if (rename(c_path.c_str(), n_path.c_str())  == 0)
        {
            path(n_path);
            add_flag('S');
        }
    }
    else
    {
        /**
         * The file is new, but not in the new folder.
         *
         * That means we need to remove "N" from the flag-component of the path.
         *
         */
        remove_flag('N');
        add_flag('S');
    }
}



/**
 * Parse the message into MIME-parts, if we've not already done so.
 */
std::vector < CMessagePart * >CMessage::get_parts()
{

    /**
     * If we've already parsed then return the cached results.
     *
     * A message can't/won't change under our feet.
     */
    if (m_parts.size() > 0)
        return (m_parts);


    /**
     * Boiler variables.
     */
    GMimeMessage * m_message;
    GMimeParser * parser;
    GMimeStream * stream;
    int fd;

    if ((fd = open(m_path.c_str(), O_RDONLY, 0)) == -1)
    {
        std::cout << "Opening failed ..." << std::endl;
        return m_parts;
    }

    stream = g_mime_stream_fs_new(fd);

    parser = g_mime_parser_new_with_stream(stream);
    g_object_unref(stream);

    m_message = g_mime_parser_construct_message(parser);
    g_object_unref(parser);

    /**
     * Create an iterator
     */
    GMimePartIter *
    iter = g_mime_part_iter_new((GMimeObject *) m_message);
    int
    count = 1;

    /**
     * Iterate over the message.
     */
    do
    {
        GMimeObject *
        part = g_mime_part_iter_get_current(iter);

        if ((GMIME_IS_OBJECT(part)) && (GMIME_IS_PART(part)))
        {
            /**
             * Get the content-type
             */
            GMimeContentType *
            content_type = g_mime_object_get_content_type(part);

            /**
             * Get the filename
             */
            const char *
            filename =
                g_mime_object_get_content_disposition_parameter(part,
                        "filename");
            gchar *
            type = g_mime_content_type_to_string(content_type);

            /**
             * Get the content.
             */
            GMimeDataWrapper *
            c = g_mime_part_get_content_object(GMIME_PART(part));
            GMimeStream *
            memstream = g_mime_stream_mem_new();
            gint64
            len = g_mime_data_wrapper_write_to_stream(c, memstream);
            guint8 *
            b =
                g_mime_stream_mem_get_byte_array((GMimeStreamMem *)
                                                 memstream)->data;

            /**
             * Copy the content away.
             */
            void *
            data = (void *) malloc(len + 1);
            size_t data_len = (size_t) len;
            memcpy(data, b, len);



            /**
             * OK this is an attachment.
             */
            if (filename)
            {
                CMessagePart *
                attach =
                    new CMessagePart(type, filename, data, data_len);
                m_parts.push_back(attach);
            }
            else
            {
                CMessagePart *
                part = new CMessagePart(type, "", data, data_len);
                m_parts.push_back(part);
            }
        }

        count += 1;
    }
    while (g_mime_part_iter_next(iter));

    g_mime_part_iter_free(iter);
    g_object_unref(m_message);


    return (m_parts);
}
