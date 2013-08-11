/**
 * file.cc - Simple file primitives.
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

#include <cstdlib>
#include <iostream>
#include <fstream>
#include <sstream>
#include <algorithm>
#include <iterator>
#include <cstdlib>
#include <string.h>
#include <iostream>
#include <sys/stat.h>
#include <sys/types.h>
#include <dirent.h>
#include <unistd.h>


#include "debug.h"
#include "file.h"
#include "maildir.h"

#ifndef FILE_READ_BUFFER
# define FILE_READ_BUFFER 16384
#endif



/**
 * Test if a file exists.
 */
bool CFile::exists( std::string path )
{
    struct stat sb;

    if ((stat(path.c_str(), &sb) == 0))
        return true;
    else
        return false;
}


/**
 * Is the given file executable?
 */
bool CFile::executable( std::string path )
{
    struct stat sb;

    /**
     * Fail to stat?  Then not executable.
     */
    if (stat(path.c_str(), &sb) < 0)
        return false;

    /**
     * If directory then not executable.
     */
    if (S_ISDIR(sb.st_mode))
        return false;

    /**
     * Otherwise check the mode-bits
     */
    if ((sb.st_mode & S_IEXEC) != 0)
        return true;
    return false;
}


/**
 * Is the given path a directory?
 */
bool CFile::is_directory(std::string path)
{
    struct stat sb;

    if (stat(path.c_str(), &sb) < 0)
      return false;

    return (S_ISDIR(sb.st_mode));
}


/**
 * Remove a file.
 */
bool CFile::delete_file(std::string path)
{
#ifdef LUMAIL_DEBUG
    std::string dm = "CFile::delete_file(\"";
    dm += path;
    dm += "\");";
    DEBUG_LOG( dm );
#endif

    /**
     * Ensure we're not removing a directory.
     */
    assert( ! CFile::is_directory( path ) );

    /**
     * Unlink.
     */
    bool result = unlink( path.c_str() );

    /**
     * Test that the file was removed.
     */
    assert( ! CFile::exists( path ) );

    return( result );
}


/**
 * Copy a file.
 */
void CFile::copy( std::string src, std::string dst )
{

#ifdef LUMAIL_DEBUG
    std::string dm = "CFile::copy(\"";
    dm += src ;
    dm += "\",\"";
    dm += dst;
    dm += "\");";
    DEBUG_LOG( dm );
#endif

    std::ifstream isrc(src, std::ios::binary);
    std::ofstream odst(dst, std::ios::binary);

    std::istreambuf_iterator<char> begin_source(isrc);
    std::istreambuf_iterator<char> end_source;
    std::ostreambuf_iterator<char> begin_dest(odst);

    std::copy(begin_source, end_source, begin_dest);

    isrc.close();
    odst.close();

    assert( CFile::exists( dst ) );
}


/**
 * Move a file.
 */
bool CFile::move( std::string src, std::string dst )
{
#ifdef LUMAIL_DEBUG
    std::string dm = "CFile::move(\"";
    dm += src ;
    dm += "\",\"";
    dm += dst;
    dm += "\");";
    DEBUG_LOG( dm );
#endif

    int ret = rename( src.c_str(), dst.c_str() );

    assert( CFile::exists(dst) );
    assert( !CFile::exists(src) );

    return( ret == 0 );
}


/**
 * Send the contents of a file to the given command, via popen.
 */
bool CFile::file_to_pipe( std::string src, std::string cmd )
{

#ifdef LUMAIL_DEBUG
    std::string dm = "CFile::file_to_pipe(\"";
    dm += src ;
    dm += "\",\"";
    dm += cmd;
    dm += "\");";
    DEBUG_LOG( dm );
#endif

    char buf[FILE_READ_BUFFER] = { '\0' };
    ssize_t nread;

    FILE *file = fopen( src.c_str(), "r" );
    if ( file == NULL )
        return false;

    FILE *pipe = popen( cmd.c_str(), "w" );
    if ( pipe == NULL )
        return false;

    /**
     * While we read from the file send to pipe.
     */
    while( (nread = fread( buf, sizeof(char), sizeof buf-1, file) ) > 0 )
    {
        char *out_ptr = buf;
        ssize_t nwritten;

        do
        {
            nwritten = fwrite(out_ptr, sizeof(char), nread, pipe);

            if (nwritten >= 0)
            {
                nread -= nwritten;
                out_ptr += nwritten;
            }
        } while (nread > 0);

        memset(buf, '\0', sizeof(buf));
    }

    pclose( pipe );
    fclose( file );

    return true;
}


/**
 * Return a sorted list of maildirs beneath the given prefix.
 */
std::vector<std::string> CFile::get_all_maildirs(std::string prefix)
{
    std::vector<std::string> result;
    dirent *de;
    DIR *dp;

    prefix = prefix.empty()? "." : prefix.c_str();
    dp = opendir(prefix.c_str());
    if (dp)
    {
        if (CMaildir::is_maildir(prefix))
            result.push_back(prefix);

        while (true)
        {
            de = readdir(dp);
            if (de == NULL)
                break;

            if ( ( strcmp( de->d_name, "." ) != 0 ) &&
                 ( strcmp( de->d_name, ".." ) != 0 ) &&
                 ( ( de->d_type == DT_UNKNOWN) ||
                   ( de->d_type == DT_DIR ) ) )
            {
                std::string subdir_name = std::string(de->d_name);
                std::string subdir_path = std::string(prefix + "/" + subdir_name);
                if (CMaildir::is_maildir(subdir_path))
                    result.push_back(subdir_path);
                else
                {
                    if (subdir_name != "." && subdir_name != "..")
                    {
                        DIR* sdp = opendir(subdir_path.c_str());
                        if (sdp)
                        {
                            closedir(sdp);
                            std::vector<std::string> sub_maildirs;
                            sub_maildirs = CFile::get_all_maildirs(subdir_path);
                            std::vector<std::string>::iterator it;
                            for (it = sub_maildirs.begin(); it != sub_maildirs.end(); ++it)
                            {
                                result.push_back(*it);
                            }
                        }
                    }
                }
            }
        }
        closedir(dp);
        std::sort(result.begin(), result.end());
    }

    return result;
}
