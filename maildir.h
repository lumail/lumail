/**
 * maildir.h - Utility functions for working with Maildirs
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

#ifndef _maildir_h_
#define _maildir_h_ 1

#include <vector>
#include <string>

/**
 * Forward decleration of class.
 */
class CMessage;


/**
 * An object for working with maildir folders.
 *
 * Opening them, counting messages, etc.
 */
class CMaildir
{
 public:

  /**
   * Constructor.  NOP.
   */
  CMaildir(std::string path);

  /**
   * Destructor.  NOP.
   */
  ~CMaildir();

  /**
   * The number of new messages for this directory.
   */
  int newMessages();

  /**
   * The number of read messages for this directory.
   */
  int availableMessages();

  /**
   * The friendly name of the maildir.
   */
  std::string name();

  /**
   * The full path to the folder.
   */
  std::string path();

  /**
   * Is the given path a Maildir?
   */
  static bool is_maildir(std::string path);

  /**
   * Is the given path a directory?
   */
  static bool is_directory(std::string path);

  /**
   * Return a sorted list of maildirs beneath the given path.
   */
  static std::vector < std::string > getFolders(std::string path);

  /**
   * Get each message in the folder.
   */
  std::vector <CMessage *> getMessages();

  /**
   * Count files in a directory.
   */
  static int countFiles(std::string path);

 private:

  /**
   * The path to the directory we represent.
   */
  std::string m_path;

};

#endif				/* _maildir_h_ */
