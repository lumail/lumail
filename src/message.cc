/**
 * message.cc - A class for working with a single message.
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

#include <algorithm>
#include <stdint.h>
#include <cstdlib>
#include <iostream>
#include <iostream>
#include <fstream>
#include <sstream>
#include <fcntl.h>
#include <string>
#include <unistd.h>
#include <pcrecpp.h>

#include <sys/stat.h>
#include <time.h>

#include "debug.h"
#include "file.h"
#include "lua.h"
#include "message.h"
#include "global.h"



/**
 * Constructor.
 */
CMessage::CMessage(std::string filename)
{
    m_path         = filename;
    m_date         = 0;
    m_time_cache   = 0;
    m_read         = false;
    m_message      = NULL;

#ifdef LUMAIL_DEBUG
    std::string dm = "CMessage::CMessage(";
    dm += m_path;
    dm += ");";
    DEBUG_LOG( dm );
#endif
}


/**
 * Destructor.
 */
CMessage::~CMessage()
{
    close_message();


#ifdef LUMAIL_DEBUG
    std::string dm = "CMessage::~CMessage(";
    dm += m_path;
    dm += ");";
    DEBUG_LOG( dm );
#endif

}


/**
 * Parse the message.
 *
 * This will use the Lua-defined `mail_filter` if it is defined.
 */
void CMessage::message_parse()
{
    if ( m_message != NULL )
        return;


    /**
     * See if we're filtering the body.
     */
    CGlobal     *global = CGlobal::Instance();
    std::string *filter = global->get_variable("mail_filter");
    std::string *tmp    = global->get_variable("tmp");

    if ( ( filter != NULL ) && ( ! ( filter->empty() ) ) )
    {
        /**
         * Generate a temporary file for the filter output.
         */
        char filename[256] = { '\0' };
        snprintf( filename, sizeof(filename)-1, "%s/body.filter.XXXXXX", tmp->c_str() );

        /**
         * Open the file.
         */
        int fd  = mkstemp(filename);

        /**
         * Avoid "unused variable" warning.
         */
        (void)(fd);

        /**
         * Build up the command to execute, via cat.
         */
        std::string cmd = "/bin/cat" ;
        assert( CFile::exists( cmd ) );

        cmd += " ";
        cmd += path();
        cmd += "|";
        cmd += *filter;

        /**
         * Run through the popen dance.
         */
        FILE* pipe = popen(cmd.c_str(), "r");
        if (pipe)
        {
            char buffer[16384] = { '\0' };
            std::string tmp = "";

            while(!feof(pipe))
            {
                if(fgets(buffer, sizeof(buffer)-1, pipe) != NULL)
                    tmp += buffer;

                memset(buffer, '\0', sizeof(buffer));
            }
            pclose(pipe);

            /**
             * Write the body out to disk.
             */
            std::ofstream on;
            on.open(filename, std::ios::binary);
            on.write(tmp.c_str(), tmp.size());
            on.close();

            /**
             * Parse the message, from the temporary file.
             */
            open_message( filename );
            /**
             * Don't leak the filename
             */
            CFile::delete_file( filename );
            return;
        }
    }

    /**
     * OK we've not parsed the message, and there is not filter present.
     * so parse the literal message.
     */
    open_message( path().c_str() );
}



/**
 * Get the path to the message on-disk.
 */
std::string CMessage::path()
{
    return (m_path);
}


/**
 * Update the path to the message.
 */
void CMessage::path( std::string new_path )
{
    m_path = new_path;

    /**
     * Reset the cached stat() data.
     */
    m_time_cache = 0;
}


/**
 * Get the flags for this message.
 */
std::string CMessage::flags()
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
 * Add a flag to a message.
 */
void CMessage::add_flag( char c )
{
    /**
     * Flags are upper-case.
     */
    c = toupper(c);

    /**
     * If the flag is already present, return.
     */
    if ( has_flag( c ) )
        return;

    /**
     * Get the path and ensure it is present.
     */
    std::string orig = path();
    std::string p( orig );

    if (p.empty())
        return;

    /**
     * Get the current flags.
     */
    size_t offset;

    std::string flags;

    if ( ( offset =  p.find(":2,") ) != std::string::npos )
    {
        flags = p.substr( offset+3  /* strlen( ":2," ) */ );
        p     = p.substr(0, offset);
    }

#ifdef LUMAIL_DEBUG
    std::string dm = "CMessage::add_flag(\"";
    dm += "Path:";
    dm += p;
    dm += " ->( p:" + p;
    dm += " f:" + flags;
    dm += ");";
    DEBUG_LOG( dm );
#endif


    /**
     * Given an input path like:
     *
     * /home/skx/Maildir/.247blinds.co.uk/cur/1239736741.19771_.skx:2,RS
     *
     * We now have:
     *
     * p     = /home/skx/Maildir/.247blinds.co.uk/cur/1239736741.19771_.skx
     *
     * flags = RS
     */

    /**
     * Add the flag to the component.
     */
    flags += c;

    /**
     * Sleazy Hack.
     */
    if ( p.find( "/new/" ) != std::string::npos )
        flags += "N";

    /**
     * Sort the flags, and remove duplicate entries.
     */
    std::sort( flags.begin(), flags.end());
    flags.erase(std::unique(flags.begin(), flags.end()), flags.end());


    /**
     * Now rename to : $path:2,$flags
     */
    std::string dst = p + ":2," + flags;

    CFile::move( orig, dst );


    /**
     * Update the path.
     */
    path( dst );
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

    if ( flags().find( c ) != std::string::npos)
        return true;
    else
        return false;
}

/**
 * Remove a flag from a message.
 */
void CMessage::remove_flag( char c )
{
    /**
     * Flags are upper-case.
     */
    c = toupper(c);

    /**
     * If the flag is not present, return.
     */
    if ( flags().find( c ) == std::string::npos)
        return;

    /**
     * Get the path and ensure it is present.
     */
    std::string p = path();

    if (p.empty())
        return;

    /**
     * Find the flags.
     */
    size_t offset     = p.find(":2,");
    std::string base  = p;
    std::string flags = "";

    /**
     * If we found flags then add the new one.
     */
    if (offset != std::string::npos)
    {
        flags = p.substr(offset);
        base  = p.substr(0,offset);
    }
    else
    {
        flags = ":2,";
    }

    /**
     * Remove the flag.
     */
    std::string::size_type k = 0;
    while((k=flags.find(c,k))!=flags.npos)
        flags.erase(k, 1);

    /**
     * Move the file.
     */
    CFile::move( p, base + flags );

    /**
     * Update the path.
     */
    path( base + flags );
}


/**
 * Does this message match the given filter?
 */
bool CMessage::matches_filter( std::string *filter )
{
    if ( strcmp( filter->c_str(), "all" ) == 0 )
        return true;

    if ( strcmp( filter->c_str(), "new" ) == 0 )
    {
        if ( is_new() )
            return true;
        else
            return false;
    }

    /**
     * Is this a header-limit?
     */
    if ( filter->length() > 8 && strncasecmp( filter->c_str(), "HEADER:", 7 ) == 0 )
    {
        /**
         * Find the ":" to split the value.
         */
        size_t offset = filter->find( ":", 8 );
        if ( offset != std::string::npos )
        {
            /**
             * The header we're matching and the pattern against which to limit.
             */
            std::string head    = filter->substr(7,offset-7);
            std::string pattern = filter->substr(offset+1);

            /**
             * Split the header list by "|" and return true if any of
             * them match.
             */
            std::istringstream helper(head);
            std::string tmp;
            while (std::getline(helper, tmp, '|'))
            {
                std::string value = header( tmp );

                if (pcrecpp::RE(pattern, pcrecpp::RE_Options().set_caseless(true)).PartialMatch(value) )
                    return true;
            }
            return false;

        }
    }

    /**
     * OK now we're falling back to matching against the formatted version
     * of the message - as set by `index_format`.
     */
    std::string formatted = format();

    /**
     * Regexp Matching.
     */
    if (pcrecpp::RE(*filter, pcrecpp::RE_Options().set_caseless(true)).PartialMatch(formatted) )
        return true;

    return false;
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
    if ( ( has_flag( 'N' ) ) || ( ! has_flag( 'S' ) ) )
        return true;

    return false;
}


/**
 * Get the message last modified time (cached).
 */
const time_t CMessage::mtime()
{
    struct stat s;

    if (m_time_cache != 0)
    {
        return m_time_cache;
    }

    if (stat(path().c_str(), &s) < 0)
        return m_time_cache;

    memcpy(&m_time_cache, &s.st_mtime, sizeof(time_t));
    return m_time_cache;
}


/**
 * Mark the message as read.
 */
bool CMessage::mark_read()
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
    if ( ( offset = c_path.find( "/new/" ) )!= std::string::npos )
    {
        /**
         * Path component before /new/ + after it.
         */
        std::string before = c_path.substr(0,offset);
        std::string after  = c_path.substr(offset+strlen("/new/"));

        n_path = before + "/cur/" + after;
        if ( rename(  c_path.c_str(), n_path.c_str() )  == 0 )
        {
            path(n_path);
            add_flag( 'S' );
            return true;
        }
        else
        {
            return false;
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
        remove_flag( 'N' );
        add_flag( 'S' );
        return true;
    }
}


/**
 * Mark the message as unread.
 */
bool CMessage::mark_unread()
{
    if ( has_flag( 'S' ) )
    {
        remove_flag( 'S' );
        return true;
    }

    return false;
}


/**
 * Format the message for display in the header - via the lua format string.
 */
std::string CMessage::format( std::string fmt )
{
    std::string result = fmt;

    /**
     * Get the format-string we'll expand from the global
     * setting, if it wasn't supplied.
     */
    if ( result.empty() )
    {
        CGlobal *global  = CGlobal::Instance();
        std::string *fmt = global->get_variable("index_format");
        result = std::string(*fmt);
    }

    /**
     * The variables we know about.
     */
    const char *fields[10] = { "FLAGS", "FROM", "TO", "SUBJECT",  "DATE", "YEAR", "MONTH", "MON", "DAY", 0 };
    const char **std_name = fields;


    /**
     * Iterate over everything we could possibly-expand.
     */
    for( int i = 0 ; std_name[i] ; ++i)
    {

        size_t offset = result.find( std_name[i], 0 );

        if ( ( offset != std::string::npos ) && ( offset < result.size() ) )
        {

            /**
             * The bit before the variable, the bit after, and the body we'll replace it with.
             */
            std::string before = result.substr(0,offset-1);
            std::string body = "";
            std::string after  = result.substr(offset+strlen(std_name[i]));

            /**
             * Expand the specific variables.
             */
            if ( strcmp(std_name[i] , "TO" ) == 0 )
            {
                body = header( "To" );
            }
            if ( strcmp(std_name[i] , "DATE" ) == 0 )
            {
                body = date();
            }
            if ( strcmp(std_name[i] , "FROM" ) == 0 )
            {
                body += header( "From" );
            }
            if ( strcmp(std_name[i] , "FLAGS" ) == 0 )
            {
                /**
                 * Ensure the flags are suitably padded.
                 */
                body = flags();

                while( body.size() < 4 )
                    body += " ";
            }
            if ( strcmp(std_name[i] , "SUBJECT" ) == 0 )
            {
                body = header( "Subject" );
            }
            if ( strcmp(std_name[i],  "YEAR" ) == 0 )
            {
                body = date(EYEAR);
            }
            if ( strcmp(std_name[i],  "MONTH" ) == 0 )
            {
                body = date(EMONTH);
            }
            if ( strcmp(std_name[i],  "MON" ) == 0 )
            {
                body = date(EMON);
            }
            if ( strcmp(std_name[i],  "DAY" ) == 0 )
            {
                body = date(EDAY);
            }

            result = before + body + after;
        }
    }

    /**
     * If the value is still unchanged ..
     */
    if ( result.size() > 1 && result.at(0) == '$' )
    {
        /**
         * See if it is header value we can find.
         */
        result = header( result.substr(1) );
        if ( result.empty() )
            result = "[unset]";
    }
    return( result );
}


/**
 * Retrieve the value of a given header from the message.
 */
std::string CMessage::header( std::string name )
{
    /**
     * Ensure the message has been read.
     */
    message_parse();

    /**
     * The result.
     */
    std::string result;

    /**
     * Get the header.
     */
    const char *str = g_mime_object_get_header ((GMimeObject *) m_message, name.c_str() );

    /**
     * If that succeeded, decode it.
     */
    if ( str != NULL )
    {
        char *decoded = g_mime_utils_header_decode_text ( str );

#ifdef LUMAIL_DEBUG
        std::string dm = "CMessage::header('";
        dm +=  (( str != NULL ) ? str : "NULL" );
        dm += "') -> '";
        dm +=  (( decoded != NULL ) ? decoded : "NULL" );
        dm += "'" ;
        DEBUG_LOG( dm );
#endif


        result = decoded;

        g_free (decoded);
    }
    return( result );
}



/**
 * Get the date of the message.
 */
std::string CMessage::date(TDate fmt)
{
    /**
     * If we have a date setup, then use it.
     */
    if ( m_date == 0 )
    {
        /**
         * Get the header.
         */
        std::string date = header("Date");

        if ( date.empty() )
        {
            /**
             * The date was empty, so use the mtime.
             */
            m_date = mtime();
        }
        else
        {
            struct tm t;

            /**
             * Find the date-formats we accept.
             *
             * Any from lua.
             */
            CLua *lua = CLua::Instance();
            std::vector<std::string> fmts = lua->table_to_array( "date_formats" );

            /**
             * Add in the ones we know about too.
             */
            fmts.push_back( "%a, %d %b %y %H:%M:%S" );
            fmts.push_back( "%a, %d %b %Y %H:%M:%S" );
            fmts.push_back( "%a, %d %b %y %H:%M:%S %z" );
            fmts.push_back( "%a, %d %b %Y %H:%M:%S %z" );
            fmts.push_back( "%d %b %y %H:%M:%S" );
            fmts.push_back( "%d %b %Y %H:%M:%S" );
            fmts.push_back( "%a %b %d %H:%M:%S GMT %Y" );
            fmts.push_back( "%a %b %d %H:%M:%S MSD %Y" );
            fmts.push_back( "%a %b %d %H:%M:%S BST %Y" );
            fmts.push_back( "%a %b %d %H:%M:%S CEST %Y" );
            fmts.push_back( "%a %b %d %H:%M:%S PST %Y" );
            fmts.push_back( "%a, %d %b %y %H:%M" );
            fmts.push_back( "%a, %d %b %Y %H:%M" );
            fmts.push_back( "%a, %d %b %Y %H.%M.%S" );
            fmts.push_back( "%d-%b-%Y" );
            fmts.push_back( "%m/%d/%y" );
            fmts.push_back( "%d %b %Y" );
            fmts.push_back( "%a %b %d %H:%M:%S %Y" );

            char* rc = NULL;

            char *current_loc = NULL;

            current_loc = setlocale(LC_TIME, NULL);

            if (current_loc != NULL)
            {
                current_loc = strdup(current_loc);
                setlocale(LC_TIME, "C");
            }

            /**
             * For each format.
             */
            std::vector<std::string>::iterator it;
            for (it = fmts.begin(); it != fmts.end(); ++it)
            {
                if ( rc )
                    break;

                const char* fmt = (*it).c_str();

                t.tm_sec=0;
                t.tm_min=0;
                t.tm_hour=0;
                rc = strptime(date.c_str(), fmt, &t);
            }

            if ( current_loc != NULL )
            {
                setlocale(LC_TIME, current_loc);
                free(current_loc);
            }

            if (!rc)
            {
                /**
                 * Failed to find a date.
                 */
                m_date = -1;

                /**
                 * Prepare an error message.
                 */
                std::string error = "alert(\"Failed to parse date: " + date + "\", 30 );" ;
                CLua *lua = CLua::Instance();
                lua->execute( error );

                /**
                 * Return the unmodified string which is the best we can hope for.
                 */
                return( date );
            }
            char tzsign[2];
            unsigned int tzhours;
            unsigned int tzmins;
            int tzscan = sscanf(rc," %1[+-]%2u%2u",tzsign,&tzhours,&tzmins);
            if (tzscan==3)
            {
                switch(tzsign[0])
                {
                case '+':
                    t.tm_hour -= tzhours;
                    t.tm_min -= tzmins;
                    break;
                case '-':
                    t.tm_hour += tzhours;
                    t.tm_min += tzmins;
                    break;
                }
            }
            else
            {
                /**
                 * Warning, couldn't parse timezone.  Probably "BST" or "EST" or
                 * something like that.  Ignore it.
                 */
            }

            /**
             *  Note: the following line used to use mktime(), until summer time
             * started and everything went off by an hour.  This timezone stuff
             * is unpleasant.
             */
            m_date = timegm(&t);
        }
    }

    if ( fmt == EFULL )
    {
        std::string date = header("Date");
        return( date );
    }

    /**
     * Convert to a time.
     */
    struct tm * ptm;
    ptm = gmtime ( &m_date );


    /**
     * Year number.
     */
    if ( fmt == EYEAR )
    {
        if ( ( m_date != 0 ) && ( m_date != -1 ) )
        {
            char buff[20] = { '\0' };
            snprintf(buff, sizeof(buff)-1, "%d", ( 1900 + ptm->tm_year ) );
            return( std::string(buff) );
        }
        else
            return std::string("$YEAR");
    }

    /**
     * Month name.
     */
    if ( ( fmt == EMONTH ) || ( fmt == EMON ) )
    {
        const char *months[] = { "January",
                                 "February",
                                 "March",
                                 "April",
                                 "May",
                                 "June",
                                 "July",
                                 "August",
                                 "September",
                                 "October",
                                 "November",
                                 "December" };
        if ( ( m_date != 0 ) && ( m_date != -1 ) )
        {
            std::string month = months[ptm->tm_mon];

            if ( fmt == EMON )
                month = month.substr(0,3);

            return( month );
        }
        else
            return std::string("$MONTH");
    }

    /**
     * Day of month.
     */
    if ( fmt == EDAY )
    {
        if ( ( m_date != 0 ) && ( m_date != -1 ) )
        {
            char buff[20] = { '\0' };
            snprintf(buff, sizeof(buff)-1, "%d", ( ptm->tm_mday ) );
            return( std::string(buff) );
        }
        else
            return std::string("$DAY");
    }

    return std::string("$FAIL");
}



/**
 * This is solely used for sorting by message-headers
 */
time_t CMessage::get_date_field()
{
    if ( m_date != 0 )
        return m_date;

    /**
     * Date wasn't cached.  Make it so.
     */
    std::string tmp = date();

    /**
     * Avoid "unused variable" warning.
     */
    (void)(tmp);

    return( m_date );
}


/**
 * Get the body from our message, using GMime.
 */
std::string CMessage::get_body()
{
    /**
     * Parse the message, if not yet done.
     */
    if ( m_message == NULL )
        message_parse();

    /**
     * The body we'll return back to the caller.  May be empty if there
     * is no text/plain part in the message.
     */
    std::string result;


    /**
     * Create an iterator to walk over the MIME-parts of the message.
     */
    GMimePartIter *iter =  g_mime_part_iter_new ((GMimeObject *) m_message);
    const char *content = NULL;

    /**
     * Iterate over the message.
     */
    do
    {
        GMimeObject *part  = g_mime_part_iter_get_current (iter);

        if ( ( GMIME_IS_OBJECT( part ) ) &&
             ( GMIME_IS_PART(part) ) )
        {

            /**
             * Get the content-type
             */
            GMimeContentType *content_type = g_mime_object_get_content_type (part);

            /**
             * If the content-type is NULL then text/plain is implied.
             *
             * If the content-type is text/plain AND we don't yet have any content
             * then we can try to get it from this part.
             *
             */
            if ( ( ( content_type == NULL ) ||
                   ( g_mime_content_type_is_type (content_type, "text", "plain") ) ) &&
                 ( content == NULL ) )
            {

                /**
                 * We'll use iconv to conver the content to UTF-8 if that is
                 * not already the correct set.
                 */
                const char *charset;
                char *converted;
                gint64 len;

                /**
                 * Get the content, and setup a memory-stream to read it.
                 */
                GMimeDataWrapper *c    = g_mime_part_get_content_object( GMIME_PART(part) );
                GMimeStream *memstream = g_mime_stream_mem_new();

                /**
                 * Get the size + data.
                 */
                len = g_mime_data_wrapper_write_to_stream( c, memstream );
                guint8 *b = g_mime_stream_mem_get_byte_array((GMimeStreamMem *)memstream)->data;

                /**
                 * If we have a character set, and it isn't UTF-8 ...
                 */
                if ( (charset = g_mime_content_type_get_parameter(content_type, "charset")) != NULL &&
                     (strcasecmp(charset, "utf-8") != 0))
                {

                    /**
                     * Convert it.
                     */
                    iconv_t cv;

                    cv = g_mime_iconv_open ("UTF-8", charset);
                    converted = g_mime_iconv_strndup(cv, (const char *) b, len );
                    if (converted != NULL)
                    {
                        /**
                         * If that succeeded update our return value with it.
                         */
                        result = (const char*)converted;
                        g_free(converted);
                    }
                    else
                    {
                        /**
                         * The conversion failed; but if we have data return
                         * it regardless.
                         */
                        if ( b != NULL )
                            result = (const char *)b;
                    }
                    g_mime_iconv_close(cv);
                }
                else
                {
                    /**
                     * No character set found, or it is already UTF-8.
                     *
                     * Save the result.
                     */
                    if ( b != NULL )
                        result = ((const char *)b );
                }
                g_mime_stream_close(memstream);
                g_object_unref(memstream);
            }
        }
    }
    while (g_mime_part_iter_next (iter));

    /**
     * Cleanup.
     */
    g_mime_part_iter_free (iter);


    /**
     * If the result is empty then we'll just revert to reading the
     * message body, and returning that.
     *
     * This can happen if:
     *
     *  * There is no text/plain part of the message.
     *  * The message is bogus.
     *
     */
    if ( result.empty() )
    {

        bool in_header = true;

        std::ifstream input ( path() );
        if ( input.is_open() )
        {
            while( input.good() )
            {
                std::string line;
                getline( input, line );

                if ( in_header )
                {
                    if ( line.length() <= 0 )
                        in_header = false;
                }
                else
                {
                    result += line;
                    result += "\n";
                }

            }
            input.close();
        }
    }

    /**
     * All done.
     */
    return( result );
}


/**
 * Get the body of the message, as a vector of lines.
 */
std::vector<std::string> CMessage::body()
{
    std::vector<std::string> result;

    /**
     * Ensure the message has been read.
     */
    message_parse();


    /**
     * Attempt to get the body from the message as one
     * long line.
     */
    std::string body = get_body();

    /**
     * At this point we have a std::string containing the body.
     *
     * If we have a message_filter set then we should pipe this
     * through it.
     *
     */
    CGlobal     *global = CGlobal::Instance();
    std::string *filter = global->get_variable("display_filter");
    std::string *tmp    = global->get_variable("tmp");

    if ( ( filter != NULL ) && ( ! ( filter->empty() ) ) )
    {
        /**
         * Generate a temporary file for the filter output.
         */
        char filename[256] = { '\0' };
        snprintf( filename, sizeof(filename)-1, "%s/msg.filter.XXXXXX", tmp->c_str() );

        /**
         * Open the file.
         */
        int fd  = mkstemp(filename);

        /**
         * Avoid "unused variable" warning.
         */
        (void)(fd);

        std::ofstream on;
        on.open(filename, std::ios::binary);
        on.write(body.c_str(), body.size());
        on.close();

        /**
         * Build up the command to execute, via cat.
         */
        std::string cmd = "/bin/cat" ;
        assert( CFile::exists( cmd ) );

        cmd += " ";
        cmd += filename;
        cmd += "|";
        cmd += *filter;

        /**
         * Run through the popen dance.
         */
        FILE* pipe = popen(cmd.c_str(), "r");
        if (pipe)
        {
            char buffer[16384] = { '\0' };
            std::string tmp = "";

            while(!feof(pipe))
            {
                if(fgets(buffer, sizeof(buffer)-1, pipe) != NULL)
                    tmp += buffer;

                memset(buffer, '\0', sizeof(buffer));
            }
            pclose(pipe);

            /**
             * Replace the body we previously generated
             * with that we've read from popen.
             */
            body = tmp;
        }

        /**
         * Don't leak the temporary file.
         */
        CFile::delete_file( filename );
    }

    /**
     * Split the body into an array, by newlines.
     */
    std::stringstream stream(body);
    std::string line;
    while (std::getline(stream, line))
    {
        result.push_back( line );
    }

    return(result);
}


/**
 * Get the names of attachments to this message.
 */
std::vector<std::string> CMessage::attachments()
{
    std::vector<std::string> paths;

    /**
     * Ensure the message has been read.
     */
    message_parse();

    /**
     * Create an iterator
     */
    GMimePartIter *iter =  g_mime_part_iter_new ((GMimeObject *) m_message);

    /**
     * Iterate over the message.
     */
    do
    {
        GMimeObject *part  = g_mime_part_iter_get_current (iter);

        if ( ( GMIME_IS_OBJECT( part ) ) &&
             ( GMIME_IS_PART(part) ) )
        {
            const char *filename = g_mime_object_get_content_disposition_parameter(part, "filename");
            const char *name =  g_mime_object_get_content_type_parameter(part, "name");

            if ( filename != NULL )
                paths.push_back( filename );
            else
                if ( name != NULL )
                    paths.push_back( name );
        }

    }
    while (g_mime_part_iter_next (iter));

    g_mime_part_iter_free (iter);

    return( paths );
}


/**
 * Save the given attachment.
 */
bool CMessage::save_attachment( int offset, std::string output_path )
{

    /**
     * Ensure the message has been read.
     */
    message_parse();

    /**
     * Did we succeed?
     */
    bool ret = false;

    /**
     * Create an iterator
     */
    GMimePartIter *iter =  g_mime_part_iter_new ((GMimeObject *) m_message);

    /**
     * The current object number.
     */
    int count = 1;

    /**
     * Iterate over the message.
     */
    do
    {
        GMimeObject *part  = g_mime_part_iter_get_current (iter);

        /**
         * Get the filename - only one of these will succeed.
         */
        const char *filename = g_mime_object_get_content_disposition_parameter(part, "filename");
        const char *name =  g_mime_object_get_content_type_parameter(part, "name");

        /**
         * We'll set this to the filename, if one succeeded.
         */
        const char *nm = NULL;

        if ( filename != NULL )
            nm = filename;
        else
            if ( name != NULL )
                nm = name;

        /**
         * OK did we get a filename?  If so test to see if it is the correct
         * attachment number, and if so save it.
         */
        if ( nm != NULL )
        {

            /**
             * Are we saving this one?
             */
            if ( count == offset )
            {
                FILE *fp = NULL;

                if ( (fp = fopen (output_path.c_str(), "wt" ) ) == NULL )
                {
                    CLua *lua = CLua::Instance();
                    lua->execute( "alert('failed to open');" );
                }
                else
                {

                    GMimeDataWrapper *content;
                    GMimeStream *ostream;

                    content = g_mime_part_get_content_object ((GMimePart *) part);
                    if(!content) /* part is incomplete. */
                    {
                        CLua *lua = CLua::Instance();
                        lua->execute( "alert('content was incomplete');" );
                    }

                    ostream = g_mime_stream_file_new (fp);


                    g_mime_data_wrapper_write_to_stream (content, ostream);

                    g_object_unref(content);
                    g_object_unref(ostream);
                }
            }

            /**
             * We've found an attachment so bump the count.
             */
            count += 1;
        }

    }
    while (g_mime_part_iter_next (iter));

    g_mime_part_iter_free (iter);

    return( ret );
}


/**
 * This method is responsible for invoking the Lua on_read_message hook.
 *
 * We record whether this message has previously been displayed to avoid
 * triggering multiple times (i.e. once per screen refresh).
 */
bool CMessage::on_read_message()
{
    /**
     * If we've been invoked, return.
     */
    if ( m_read )
        return false;
    else
        m_read = true;

    /**
     * Call the hook.
     */
    CLua *lua = CLua::Instance();
    lua->execute( "on_read_message(\"" + path() + "\");" );

    /**
     * Hook invoked.
     */
    return true;
}


/**
 * Open & parse the message.
 */
void CMessage::open_message( const char *filename )
{
    GMimeParser *parser;
    GMimeStream *stream;
    int fd;

    if ((fd = open( filename, O_RDONLY, 0)) == -1)
        return;

    stream = g_mime_stream_fs_new (fd);

    parser = g_mime_parser_new_with_stream (stream);
    g_object_unref (stream);

    m_message = g_mime_parser_construct_message (parser);
    g_object_unref (parser);
}

/**
 * Close the message.
 */
void CMessage::close_message()
{
    if ( m_message != NULL )
    {
        g_object_unref( m_message );
        m_message = NULL;
    }
}



/**
 * Update a basic email, on-disk, to include the named attachments.
 */
void CMessage::add_attachments_to_mail(char *filename, std::vector<std::string> attachments )
{
    /**
     * When this code is called we have a file, on-disk, which contains something like:
     *
     *    To: foo@bar.com
     *    Subject: moi
     *    From: me@example.com
     *
     *    Body text..
     *    More text..
     *
     *    --
     *    Sig
     *
     *
     * We also have a vector of filenames which should be added as attachments
     * to this mail.
     *
     * If there are no attachments then we can return immediately, otherwise
     * we need to add the attachments to the mail - which means parsing it,
     * adding the attachments, and returning an (updated) file.
     *
     */


    /**
     * Simplest case - if there are no attachments we merely return.
     *
     * This means our message is non-MIME.  Ideally we shouldn't return,
     * instead we should always proceed, as this will encode our outgoing
     * headers, etc.
     *
     * For the moment I've left this early termination in place because
     * I'm loathe to make too many changes to this code while it is still
     * new.  It will become apparent pretty quickly if I've made the
     * wrong choice.
     *
     */
    if ( attachments.size() < 1 )
    {
        DEBUG_LOG( "CMessage::add_attachments_to_mail - No attachments for this message" );
        return;
    }

    GMimeMessage *message;
    GMimeParser  *parser;
    GMimeStream  *stream;
    int fd;



    if ((fd = open ( filename, O_RDONLY, 0)) == -1)
    {
        DEBUG_LOG( "CMessage::add_attachments_to_mail - Failed to open filename for reading" );
        return;
    }

    stream = g_mime_stream_fs_new (fd);

    parser = g_mime_parser_new_with_stream (stream);
    g_object_unref (stream);

    message = g_mime_parser_construct_message (parser);
    g_object_unref (parser);


    GMimeMultipart *multipart;
    GMimePart *attachment;
    GMimeDataWrapper *content;

    /**
     * Create a new multipart message.
     */
    multipart = g_mime_multipart_new();
    GMimeContentType *type = g_mime_content_type_new ("multipart", "mixed");
    g_mime_object_set_content_type (GMIME_OBJECT (multipart), type);


    GMimeContentType *new_type;
    GMimeObject *mime_part;

    mime_part = g_mime_message_get_mime_part (message);
    new_type = g_mime_content_type_new_from_string ("text/plain; charset=UTF-8");
    g_mime_object_set_content_type (mime_part, new_type);

    /**
     * first, add the message's toplevel mime part into the multipart
     */
    g_mime_multipart_add (multipart, g_mime_message_get_mime_part (message));

    /**
     * now set the multipart as the message's top-level mime part
     */
    g_mime_message_set_mime_part (message,(GMimeObject*) multipart);

    std::vector<std::string>::iterator it;
    for (it = attachments.begin(); it != attachments.end(); ++it)
    {
        std::string name = (*it);

        if ((fd = open (name.c_str(), O_RDONLY)) == -1)
        {
            DEBUG_LOG( "CMessage::add_attachments_to_mail - Failed to open attachment" );
            return;
        }

        stream = g_mime_stream_fs_new (fd);

        /**
         * the stream isn't encoded, so just use DEFAULT
         */
        content = g_mime_data_wrapper_new_with_stream (stream, GMIME_CONTENT_ENCODING_DEFAULT);

        g_object_unref (stream);

        /**
         * if you knew the mime-type of the file, you could use that instead
         * of application/octet-stream
         */
        attachment = g_mime_part_new_with_type ("application", "octet-stream");
        g_mime_part_set_content_object (attachment, content);
        g_object_unref (content);

        /**
         * set the filename?
         */
        g_mime_part_set_filename (attachment, basename (name.c_str()));

        /**
         * NOTE: We might want to base64 encode this for transport...
         *
         * NOTE: if you want o get really fancy, you could use
         * g_mime_part_get_best_content_encoding()
         * to calculate the most efficient encoding algorithm to use.
         */
        g_mime_part_set_content_encoding (attachment, GMIME_CONTENT_ENCODING_BASE64);


        /**
         * Add the attachment to the multipart
         */
        g_mime_multipart_add (multipart, (GMimeObject*)attachment);
        g_object_unref (attachment);
    }


    /**
     * now that we've finished referencing the multipart directly (the message still
     * holds it's own ref) we can unref it.
     */
     g_object_unref (multipart);

     /**
      * Output the updated message.  First pick a tmpfile.
      *
      * NOTE: We must use a temporary file.  If we attempt to overwrite the
      * input file we'll get corruption, due to GMime caching.
      */
     CGlobal *global   = CGlobal::Instance();
     std::string *tmp  = global->get_variable( "tmp" );
     char tmpfile[256] = { '\0' };
     snprintf( tmpfile, sizeof(tmpfile)-1, "%s/mytemp.XXXXXX", tmp->c_str() );

     /**
      * Write out the updated message.
      */
     FILE *f = NULL;
     if ((f = fopen ( tmpfile,"wb")) == NULL)
     {
         DEBUG_LOG( "CMessage::add_attachments_to_mail - Failed to open tmpfile" );
         return;
     }
     GMimeStream *ostream = g_mime_stream_file_new (f);
     g_mime_object_write_to_stream ((GMimeObject *) message, ostream);
     g_object_unref(ostream);

     /**
      * Now rename the temporary file over the top of the input
      * message.
      */
     CFile::delete_file( filename );
     CFile::move( tmpfile, filename );
}
