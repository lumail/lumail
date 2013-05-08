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

    unsigned offset = m_path.find(":2,");
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
 * Format the message for display in the header - via the lua format string.
 */
std::string CMessage::format()
{
  /**
   * Get the format-string we'll expand.
   */
  CGlobal *global = CGlobal::Instance();
  std::string *fmt = global->get_index_format();
  std::string result = std::string(*fmt);

  /**
   * The variables we know about.
   */
  const char *fields[6] = { "FLAGS", "FROM", "TO", "SUBJECT",  "DATE", 0 };
  const char **std_name = fields;


  /**
   * Iterate over everything we could possibly-expand.
   */
  for( int i = 0 ; std_name[i] ; ++i) {

    unsigned int offset = result.find( std_name[i], 0 );

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
 * Get the sender of the message.
 */
std::string CMessage::from()
{
  if ( m_me == NULL ) {
    ifstream file(path().c_str());
    m_me = new MimeEntity(file);
  }
  Header & h = m_me->header();
  return(h.from().str() );
}


/**
 * Get the date of the message.
 *
 * TODO: If date is empty stat() the filename.
 */
std::string CMessage::date()
{
  if ( m_me == NULL ) {
    ifstream file(path().c_str());
    m_me = new MimeEntity(file);
  }
  Header & h = m_me->header();
  if (h.hasField("date"))
    return(h.field("date").value());
  else
    return "No date";
}


/**
 * Get the recipient of the message.
 */
std::string CMessage::to()
{
  if ( m_me == NULL ) {
    ifstream file(path().c_str());
    m_me = new MimeEntity(file);
  }
  Header & h = m_me->header();
  return(h.to().str() );
}


/**
 * Get the subject of the message.
 */
std::string CMessage::subject()
{
  if ( m_me == NULL ) {
    ifstream file(path().c_str());
    m_me = new MimeEntity(file);
  }
  Header & h = m_me->header();
  return(h.subject());
}


