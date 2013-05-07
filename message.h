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

/**
 * A single message.
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
   * Get the path.
   */
  std::string path();


  /**
   * Format the message for display in the header - via the lua format string.
   */
  std::string format();

  /**
   * Get the flags for this message.
   */
  std::string flags();

 private:

  /**
   * The file we represent.
   */
  std::string m_path;

};

#endif				/* _message_h */
