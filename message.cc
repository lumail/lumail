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
#include <string>
#include <unistd.h>
#include <pcrecpp.h>
#include <mimetic/mimetic.h>

#include <sys/stat.h>
#include <time.h>

#include "debug.h"
#include "file.h"
#include "lua.h"
#include "message.h"
#include "global.h"

using namespace std;
using namespace mimetic;


/**
 * Constructor.
 */
CMessage::CMessage(std::string filename)
{
    m_path = filename;
    m_me   = NULL;
    m_date = 0;
    m_time_cache = 0;
}


/**
 * Destructor.
 */
CMessage::~CMessage()
{
    if ( m_me != NULL )
        delete( m_me );
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

    /**
     * Pad.
     */
    while( (int)strlen(flags.c_str()) < 4 )
        flags += " ";

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
     * Matching the formatted version.
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
bool CMessage::mark_new()
{
    /*
     * Get the current path, and build a new one.
     */
    std::string c_path = path();
    std::string n_path = "";

    size_t offset = std::string::npos;

    /**
     * If we find /cur/ in the path then rename to be /new/
     */
    if ( ( offset = c_path.find( "/cur/" ) )!= std::string::npos )
    {
        /**
         * Path component before /cur/ + after it.
         */
        std::string before = c_path.substr(0,offset);
        std::string after  = c_path.substr(offset+strlen("/cur/"));

        n_path = before + "/new/" + after;
        if ( rename(  c_path.c_str(), n_path.c_str() )  == 0 )
        {
            path( n_path );
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
         * The file is old, but not in the old folder.  That means we need to
         * add "N" to the flag-component of the path.
         */
        add_flag( 'N' );
        return true;
    }
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
                body = to();
            }
            if ( strcmp(std_name[i] , "DATE" ) == 0 )
            {
                body = date();
            }
            if ( strcmp(std_name[i] , "FROM" ) == 0 )
            {
                body += from();
            }
            if ( strcmp(std_name[i] , "FLAGS" ) == 0 )
            {
                body = flags();
            }
            if ( strcmp(std_name[i] , "SUBJECT" ) == 0 )
            {
                body = subject();
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
 * Decode a header-field.  The format is
 *
 * =?<charset>?<encoding>?<data>?=
 */
std::string CMessage::decode_field( std::string str )
{
    /**
     * If the string is empty, or the first characters are not "=" and "?"
     * we return unmodified.
     */
    if ( str.size() < 1 )
        return(str);

    if ( ( str.at(0) != '=' ) ||
         ( str.at(1) != '?' ) )
        return(str);

    /**
     * OK so we now need to break this down into the type.
     */
    size_t len = str.size();

    std::string charset;
    std::string encoding;
    std::string data;

    size_t i;
    for(  i = 2; i < len; i++ )
    {
        if (str.at(i) != '?' )
            charset += str.substr(i,1);
        else
            break;
    }

    i++;
    for( ; i < len; i++ )
    {
        if (str.at(i) != '?' )
            encoding += str.at(i);
        else
            break;
    }
    i++;

    for( ; i < len; i++ )
    {
        if (str.at(i) != '?' )
            data += str.at(i);
        else
            break;
    }

    std::string tail;

    if ( str.size() >= i + 3 )
        tail = str.substr(i+3);


    /**
     * Log what we found.
     */
#ifdef LUMAIL_DEBUG
    std::string input = "CMessage::decode_field(\"";
    input += str;
    input += "\");";
    DEBUG_LOG( input );

    std::string parsed = "CHARSET:'" + charset + "' ENCODING:" + encoding +
        " DATA:" + data + " TAIL:" + tail;
    DEBUG_LOG( parsed);
#endif


    mimetic::Base64::Decoder qp;
    mimetic::QP::Decoder     b64;

    std::istringstream is(data);
    std::ostringstream decoded;
    std::istreambuf_iterator<char> ibeg(is), iend;
    std::ostreambuf_iterator<char> oi(decoded);

    if ( encoding == "B" )
        decode(ibeg, iend, qp, oi);
    if ( encoding == "Q" )
        decode(ibeg, iend, b64, oi);

    return( decoded.str() + tail ) ;
}

/**
 * Get the value of a header.
 */
std::string CMessage::header( std::string name )
{
    if ( m_me == NULL )
    {
        ifstream file(path().c_str());
        m_me = new MimeEntity(file);
    }
    Header & h = m_me->header();
    if (h.hasField(name ) )
        return( decode_field( h.field(name).value() ) );
    else
        return "";
}


/**
 * Get the sender of the message.
 */
std::string CMessage::from()
{
    return( header( "From" ) );
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
            struct stat st_buf;
            const char *p = path().c_str();

            int err = stat(p,&st_buf);
            if ( !err )
            {
                m_date = st_buf.st_mtime;
            }
        }
        else
        {
#if 0
            /**
             *
             * Parse RFC822 date
             * Case in point: Mon, 15 Dec 2003 09:46:00 +0000 (GMT)
             * Would be OK with just +0000 or (GMT) but both together
             * is not acceptable.  But note that the () are actually a
             * comment.  So we strip comments.
             */
            unsigned int start = 0;
            while(1)
            {
                start = date.find('(',start);
                if (start==std::string::npos)
                    break;

                unsigned int end = date.find(')',start);
                if (end==std::string::npos)
                    break;
                date.erase(start,end+1-start);
            }
#endif


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

            const char *current_loc = NULL;

            current_loc = setlocale(LC_TIME, NULL);

            if (current_loc != NULL)
                setlocale(LC_TIME, "C");

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
                setlocale(LC_TIME, current_loc);

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
 * Get the recipient of the message.
 */
std::string CMessage::to()
{
    return( header( "To" ) );
}


/**
 * Get the subject of the message.
 */
std::string CMessage::subject()
{
    return( header( "Subject" ) );
}


/**
 * Given a MIME-part retrieve the appropriate part from it.
 */
std::string CMessage::getMimePart(mimetic::MimeEntity* pMe, std::string mtype )
{
    mimetic::Header& h = pMe->header();

    /**
     * If the header matches then return.
     */
    std::string enc = h.contentTransferEncoding().mechanism();
    std::string type= h.contentType().str();

    if ( type.find( mtype ) != std::string::npos )
    {
        std::string body = (pMe)->body();
        std::string decoded;

        if (strcasecmp(enc.c_str(), "quoted-printable" ) == 0 )
        {
            mimetic::QP::Decoder qp;
            decode(body.begin(), body.end(), qp, std::back_inserter(decoded));
            return( decoded );
        }
        if (strcasecmp(enc.c_str(), "base64" ) == 0 )
        {
            mimetic::Base64::Decoder b64;
            decode(body.begin(), body.end(), b64, std::back_inserter(decoded));
            return( decoded );
        }

        return( body );
    }

    /**
     * Iterate over any sub-parts.
     */
    mimetic::MimeEntityList& parts = pMe->body().parts();
    mimetic::MimeEntityList::iterator mbit = parts.begin(), meit = parts.end();
    for(; mbit != meit; ++mbit)
    {

        /**
         * get the encoding and content-type.
         */
        mimetic::Header& mh = (*mbit)->header();
        std::string enc = mh.contentTransferEncoding().mechanism();
        std::string type= mh.contentType().str();

        /**
         * See if the part matches directly.
         */
        if ( type.find( mtype ) != std::string::npos )
        {
            std::string body = (*mbit)->body();
            std::string decoded;

            if (strcasecmp(enc.c_str(), "quoted-printable" ) == 0 )
            {
                mimetic::QP::Decoder qp;
                decode(body.begin(), body.end(), qp, std::back_inserter(decoded));
                return( decoded );
            }
            if (strcasecmp(enc.c_str(), "base64" ) == 0 )
            {
                mimetic::Base64::Decoder b64;
                decode(body.begin(), body.end(), b64, std::back_inserter(decoded));
                return( decoded );
            }

            return( body );
        }

        /**
         * There are also nested parts because MIME is nasty.
         */
        mimetic::MimeEntityList& np = (*mbit)->body().parts();
        mimetic::MimeEntityList::iterator bit = np.begin(), eit = np.end();
        for(; bit != eit; ++bit)
        {
            /**
             * See if a nested entry matches.
             */
            mimetic::Header& nh = (*bit)->header();

            /**
             * Get the encoding-type and the content-type.
             */
            std::string enc = nh.contentTransferEncoding().mechanism();
            std::string type= nh.contentType().str();

            /**
             * If the type matches, then return the (decoded?) body
             */
            if ( type.find( mtype ) != std::string::npos )
            {
                std::string body = (*bit)->body();
                std::string decoded;

                if (strcasecmp(enc.c_str(), "quoted-printable" ) == 0 )
                {
                    mimetic::QP::Decoder qp;
                    decode(body.begin(), body.end(), qp, std::back_inserter(decoded));
                    return( decoded );
                }
                if (strcasecmp(enc.c_str(), "base64" ) == 0 )
                {

                    mimetic::Base64::Decoder b64;
                    decode(body.begin(), body.end(), b64, std::back_inserter(decoded));
                    return( decoded );
                }

                return( body );
            }
        }
    }
    return "";
}


/**
 * Get the body of the message, as a vector of lines.
 */
std::vector<std::string> CMessage::body()
{
    std::vector<std::string> result;

    /**
     * Parse if we've not done so.
     */
    if ( m_me == NULL )
    {
        ifstream file(path().c_str());
        m_me = new MimeEntity(file);
    }

    /**
     * Attempt to get the body from the message.
     *
     * Note: We handle nested MIME-entities here.
     *
     */
    std::string body = getMimePart( m_me, "text/plain" );

    /**
     * If we failed to find a part of text/plain then just grab the whole damn
     * thing and hope for the best.
     */
    if ( body.empty() )
        body = m_me->body();


    /**
     * At this point we have a std::string containing the body.
     *
     * If we have a message_filter set then we should pipe this
     * through it.
     *
     */
    CGlobal     *global = CGlobal::Instance();
    std::string *filter = global->get_variable("message_filter");
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

        ofstream on;
        on.open(filename, ios::binary);
        on.write(body.c_str(), body.size());
        on.close();

        /**
         * Build up the command to execute.
         */
        std::string cmd = "/bin/cat " ;
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
        unlink( filename );
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
     * Parse if we've not done so.
     */
    if ( m_me == NULL )
    {
        ifstream file(path().c_str());
        m_me = new MimeEntity(file);
    }

    /**
     * Iterate over every part.
     */
    mimetic::MimeEntityList& parts = m_me->body().parts();
    mimetic::MimeEntityList::iterator mbit = parts.begin(), meit = parts.end();
    for(; mbit != meit; ++mbit)
    {
        MimeEntity *p = *mbit;
        const mimetic::ContentDisposition& cd = p->header().contentDisposition();
        string fn = cd.param("filename");

        /**
         * Store the filename.
         */
        if ( ! fn.empty() )
        {
            paths.push_back( fn );
        }
    }

    return( paths );
}


/**
 * Save the given attachment.
 */
bool CMessage::save_attachment( int offset, std::string output_path )
{
    bool ret = false;

    /**
     * Parse if we've not done so.
     */
    if ( m_me == NULL )
    {
        ifstream file(path().c_str());
        m_me = new MimeEntity(file);
    }

    /**
     * Iterate over every part.
     */
    mimetic::MimeEntityList& parts = m_me->body().parts();
    mimetic::MimeEntityList::iterator mbit = parts.begin(), meit = parts.end();
    int m_off = 1;

    for(; mbit != meit; ++mbit)
    {
        MimeEntity *p = *mbit;
        const mimetic::ContentDisposition& cd = p->header().contentDisposition();
        string fn = cd.param("filename");

        if ( ! fn.empty() )
        {
            if ( m_off == offset )
            {
                std::string body = p->body();
                std::string decoded;

                /**
                 * Encoding type.
                 */
                std::string enc = p->header().contentTransferEncoding().mechanism();


                std::ofstream myfile;
                myfile.open(output_path, ios::binary|ios::out);


                if (strcasecmp(enc.c_str(), "quoted-printable" ) == 0 )
                {
                    mimetic::QP::Decoder qp;
                    decode(body.begin(), body.end(), qp, std::back_inserter(decoded));
                }
                if (strcasecmp(enc.c_str(), "base64" ) == 0 )
                {

                    mimetic::Base64::Decoder b64;
                    decode(body.begin(), body.end(), b64, std::back_inserter(decoded));
                }


                myfile << decoded;
                myfile.close();

                ret = true;
            }
            m_off += 1;
        }

    }


    return( ret );
}
