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

#include <stdint.h>
#include <iostream>
#include <iostream>
#include <fstream>
#include <sstream>
#include <string>

#include <mimetic/mimetic.h>

#include <sys/stat.h>
#include <time.h>

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
 * Get the flags for this message.
 */
std::string CMessage::flags()
{
  std::string flags = "";

  if (m_path.empty())
    return (flags);

  size_t offset = m_path.find(":2,");
  if (offset != std::string::npos)
    flags = m_path.substr(offset + 3);

  if ( flags.size() > 3)
    flags = "";

  /**
   * Sleazy Hack.
   */
  if ( m_path.find( "/new/" ) != std::string::npos )
    flags += "N";

  /**
   * Pad.
   */
  while( (int)strlen(flags.c_str()) < 4 )
    flags += " ";

  return flags;
}


/**
 * Does this message match the given filter?
 */
bool CMessage::matches_filter( std::string *filter )
{
  if ( strcmp( filter->c_str(), "all" ) == 0 )
    return true;

  if ( strcmp( filter->c_str(), "new" ) == 0 ) {
      if ( is_new() )
        return true;
      else
        return false;
  }

  /**
   * Matching the formatted version.
   */
  std::string formatted = format();
  if ( strstr( formatted.c_str(), filter->c_str() ) != NULL )
    return true;

  return false;
}


/**
 * Is this message new?
 */
bool CMessage::is_new()
{
  std::string f = flags();
  if ( f.find( 'N' ) != std::string::npos )
    return true;
  else
    return false;
}


/**
 * Mark the message as read.
 */
bool CMessage::mark_read()
{
  /*
   * Get the current path, and build a new one.
   */
  std::string c_path = m_path;
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
      if ( rename(  c_path.c_str(), n_path.c_str() )  == 0 ) {
        m_path = n_path;
        return true;
      }
      else {
        return false;
      }
    }
  else {
    /**
     * The file is new, but not in the new folder.  That means we need to remove "N" from
     * the flag-component of the path.
     *
     * TODO
     */
    return false;
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
  std::string c_path = m_path;
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
      if ( rename(  c_path.c_str(), n_path.c_str() )  == 0 ) {
        m_path = n_path;
        return true;
      }
      else {
        return false;
      }
    }
  else {
    /**
     * The file is old, but not in the old folder.  That means we need to add "N" to
     * the flag-component of the path.
     *
     * TODO
     */
    return false;
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
  if ( result.empty() ) {
    CGlobal *global  = CGlobal::Instance();
    std::string *fmt = global->get_index_format();
    result = std::string(*fmt);
  }

  /**
   * The variables we know about.
   */
  const char *fields[6] = { "FLAGS", "FROM", "TO", "SUBJECT",  "DATE", 0 };
  const char **std_name = fields;


  /**
   * Iterate over everything we could possibly-expand.
   */
  for( int i = 0 ; std_name[i] ; ++i) {

    size_t offset = result.find( std_name[i], 0 );

    if ( ( offset != std::string::npos ) && ( offset < result.size() ) ) {

      /**
       * The bit before the variable, the bit after, and the body we'll replace it with.
       */
      std::string before = result.substr(0,offset-1);
      std::string body = "";
      std::string after  = result.substr(offset+strlen(std_name[i]));

      /**
       * Expand the specific variables.
       */
      if ( strcmp(std_name[i] , "TO" ) == 0 ) {
        body = to();
      }
      if ( strcmp(std_name[i] , "DATE" ) == 0 ) {
        body = date();
      }
      if ( strcmp(std_name[i] , "FROM" ) == 0 ) {
        body += from();
      }
      if ( strcmp(std_name[i] , "FLAGS" ) == 0 ) {
        body = flags();
      }
      if ( strcmp(std_name[i] , "SUBJECT" ) == 0 ) {
        body = subject();
      }

      result = before + body + after;
    }
  }
  return( result );
}


/**
 * Get the value of a header.
 */
std::string CMessage::header( std::string name )
{
  if ( m_me == NULL ) {
    ifstream file(path().c_str());
    m_me = new MimeEntity(file);
  }
  Header & h = m_me->header();
  if (h.hasField(name ) )
    return(h.field(name).value() );
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
 *
 * TODO: ctime vs localtime
 */
std::string CMessage::date()
{
  std::string date = header("Date");

  if (date.empty()) {
    struct stat st_buf;
    const char *p = path().c_str();

    int err = stat(p,&st_buf);
    if ( !err ) {
      time_t modt = st_buf.st_mtime;
      date = ctime(&modt);
    }
  }

  return( date );
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
 * Get the body of the message, as a vector of lines.
 */
std::vector<std::string> CMessage::body()
{
  std::vector<std::string> result;

  /**
   * Parse if we've not done so.
   */
  if ( m_me == NULL ) {
    ifstream file(path().c_str());
    m_me = new MimeEntity(file);
  }

  /**
   * The body.
   */
  std::string body;

  /**
   * Iterate over every part.
   */
  mimetic::MimeEntityList& parts = m_me->body().parts();
  mimetic::MimeEntityList::iterator mbit = parts.begin(), meit = parts.end();
  for(; mbit != meit; ++mbit) {

    /**
     * Get the content-type.
     */
    std::string type = (*mbit)->header().contentType().str();

    /**
     * If we've found text/plain then we're good.
     */
    if ( type.find( "text/plain" ) != std::string::npos )
      {
        if ( body.empty() )
          body =  (*mbit)->body();
      }
  }

  /**
   * If we failed to find a part of text/plain then just grab the whole damn
   * thing and hope for the best.
   */
  if ( body.empty() )
    body = m_me->body();


  /**
   * Split the body into an array, by newlines.
   */
  std::stringstream stream(body);
  std::string line;
  while (std::getline(stream, line)) {
    result.push_back( line );
  }

  return(result);
}
