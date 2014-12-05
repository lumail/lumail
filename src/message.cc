
#include <algorithm>
#include <cstdlib>
#include <dirent.h>
#include <fcntl.h>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string.h>
#include <string>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <unordered_map>
#include <vector>


#include <gmime/gmime.h>


#include "message.h"



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
    return( h[ name ] );
}


/**
 * Return all header-names, and their values.
 */
std::unordered_map < std::string, std::string > CMessage::headers()
{
    std::unordered_map < std::string, std::string > m_header_values;

    GMimeMessage *m_message;
    GMimeParser *parser;
    GMimeStream *stream;
    int fd;

    if ((fd = open(m_path.c_str(), O_RDONLY, 0)) == -1)
        throw "Opening the message failed";

    stream = g_mime_stream_fs_new(fd);

    parser = g_mime_parser_new_with_stream(stream);
    g_object_unref(stream);

    m_message = g_mime_parser_construct_message(parser);
    g_object_unref(parser);


    const char *name;
    const char *value;

    /**
     * Prepare to iterate.
     */
    GMimeHeaderList *ls = GMIME_OBJECT(m_message)->headers;
    GMimeHeaderIter *iter = g_mime_header_iter_new();

    if (g_mime_header_list_get_iter(ls, iter) && g_mime_header_iter_first(iter))
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
            char *decoded = g_mime_utils_header_decode_text(value);
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
 * Destructor.
 */
CMessage::~CMessage()
{
}


/**
 * Retrieve the current flags for this message.
 */
std::string CMessage::get_flags()
{
    std::string flags = "";
    std::string pth   = path();

    if (pth.empty())
        return (flags);

    size_t offset = pth.find(":2,");
    if (offset != std::string::npos)
        flags = pth.substr(offset + 3);

    /**
     * Sleazy Hack.
     */
    if ( pth.find( "/new/" ) != std::string::npos )
        flags += "N";


    /**
     * Sort the flags, and remove duplicates
     */
    std::sort( flags.begin(), flags.end());
    flags.erase(std::unique(flags.begin(), flags.end()), flags.end());

    return flags;
}


/**
 * Set the flags for this message.
 */
void CMessage::set_flags( std::string new_flags )
{
    /**
     * Sort the flags.
     */
    std::string flags = new_flags;
    std::sort( flags.begin(), flags.end());
    flags.erase(std::unique(flags.begin(), flags.end()), flags.end());

    /**
     * Get the current ending position.
     */
    std::string cur_path = path();
    std::string dst_path = cur_path;

    size_t offset = std::string::npos;

    if ( ( offset =  cur_path.find(":2,") ) != std::string::npos )
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

    if ( cur_path != dst_path )
    {
        //
        //  TODO:  Rename
        //
        //CFile::move( cur_path, dst_path );
        //path( dst_path );
    }
}


/**
 * Add a flag to a message.
 *
 * Return true if the flag was added, false if already present.
 */
bool CMessage::add_flag( char c )
{
    std::string flags = get_flags();

    size_t offset = std::string::npos;

    /**
     * If the flag was missing, add it.
     */
    if ( ( offset = flags.find( c ) ) == std::string::npos )
    {
        flags += c;
        set_flags( flags );
        return true;
    }
    else
        return false;
}


/**
 * Does this message possess the given flag?
 */
bool CMessage::has_flag( char c )
{
    /**
     * Flags are upper-case.
     */
    c = toupper(c);

    if ( get_flags().find( c ) != std::string::npos)
        return true;
    else
        return false;
}

/**
 * Remove a flag from a message.
 *
 * Return true if the flag was removed, false if it wasn't present.
 */
bool CMessage::remove_flag( char c )
{
    c = toupper(c);

    std::string current = get_flags();

    /**
     * If the flag is not present, return.
     */
    if ( current.find( c ) == std::string::npos)
        return false;

    /**
     * Remove the flag.
     */
    std::string::size_type k = 0;
    while((k=current.find(c,k))!=current.npos)
        current.erase(k, 1);

    set_flags( current );

    return true;
}
