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
 * Using the mimetic library we'll parse the message and make verious fields available.
 *
 */
class CMessage
{
 public:

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
   * Format the message for display in the header - via the lua format string.
   */
  std::string format( std::string fmt = "");


  /**
   * Get the flags for this message.
   */
  std::string flags();

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
  std::string date();


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


 private:

  /**
   * The file we represent.
   */
  std::string m_path;

  /**
   * MIME Entity object for this message.
   */
  mimetic::MimeEntity *m_me;
};

#endif				/* _message_h */
