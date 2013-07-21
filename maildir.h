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
   * Constructor.
   */
  CMaildir(std::string path);

  /**
   * Destructor.  NOP.
   */
  ~CMaildir();

  /**
   * The number of new messages for this maildir.
   */
  int unread_messages();

  /**
   * The total number of messages for this maildir.
   */
  int all_messages();

  /**
   * The friendly name of the maildir.
   */
  std::string name();

  /**
   * The full path to the folder.
   */
  std::string path();

  /**
   * Format this maildir for display in maildir-mode.
   */
  std::string format( bool selected, std::string fmt = "" );

  /**
   * Does this maildir match the given filter?
   */
  bool matches_filter( std::string *filter );

  /**
   * Generate a new filename in the given folder.
   */
  static std::string message_in(std::string path, bool is_new);

  /**
   * Is the given path a Maildir?
   */
  static bool is_maildir(std::string path);

  /**
   * Get each message in the folder.
   */
  std::vector<CMessage *> getMessages();


private:

  /**
   * Return the last modified time for this Maildir.
   */
  time_t last_modified();

  /**
   * Update the cached total/unread message counts.
   */
  void update_cache();

 private:

  /**
   * The path to the directory we represent.
   */
  std::string m_path;

  /**
   * Cached time/date object.
   */
  time_t m_modified;

  /**
   * Cached unread-count + cached total count.
   */
  int m_unread;
  int m_total;
};

#endif /* _maildir_h_ */
