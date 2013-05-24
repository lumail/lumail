/**
 * global.h - Singleton interface to store global data
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

#ifndef _global_h_
#define _global_h_ 1

#include <unordered_map>
#include <string>
#include <vector>
#include "maildir.h"
#include "message.h"

/**
 * A singleton class to store global data.
 *
 */
class CGlobal
{
 public:

  /**
   * Get access to the singleton instance.
   */
  static CGlobal *Instance();

  /**
   * Get/Set the current lumail-mode: index/maildir/message
   */
  void set_mode(std::string * mode);
  std::string * get_mode();

  /**
   * Get/Set the maildir-prefix, which is where we find Maildir-folders beneath.
   */
  void set_maildir_prefix(std::string * prefix);
  std::string * get_maildir_prefix();

  /**
   * Get/Set the index-format.
   */
  void set_index_format(std::string * fmt);
  std::string * get_index_format();

  /**
   * Get all folders.
   */
  std::vector<CMaildir> get_all_folders();

  /**
   * Get all folders which match the current mode: new/all/pattern
   */
  std::vector<CMaildir> get_folders();

  /**
   * Get all selected folders.
   */
  std::vector<std::string> get_selected_folders();

  /**
   * Get all messages from the currently-selected folders.
   */
  std::vector<CMessage *> * get_messages();

  /**
   * Update the list of messages.
   */
  void update_messages();


  /**
   * Remove all selected folders.
   */
  void unset_folders();

  /**
   * Add a folder to the selected set.
   */
  void add_folder(std::string path);

  /**
   * Remove a folder from the selected set.
   */
  bool remove_folder(std::string path);

  /**
   * Get/Set the maildir-display limit: new/all/pattern
   */
  std::string * get_maildir_limit();
  void set_maildir_limit(std::string * limit);

  /**
   * Get/Set the index-limit: new/all/pattern.
   */
  std::string * get_index_limit();
  void set_index_limit(std::string * limit);

  /**
   * Get/set the selected folder, i.e. the one with the highlight.
   */
  int get_selected_folder() {
    return m_cur_folder;
  }
  void set_selected_folder(int offset) {
    m_cur_folder = offset;
  }

  /**
   * Get/Set the selected message.  Not used.
   */
  int get_selected_message() {
    return m_cur_message;
  }
  void set_selected_message(int offset) {
    m_cur_message = offset;
  }

  /**
   * Get/Set the sent-mail path.
   */
  std::string * get_sent_mail();
  void set_sent_mail( std::string *path);


  /**
   * Get/Set an arbitrary setting.
   */
  std::string * get_variable( std::string name );
  void set_variable( std::string name, std::string *value );

  /**
   * Get the table of all known settings.
   */
  std::unordered_map<std::string, std::string *> get_variables();


 protected:

  /**
   * Protected functions to allow our singleton implementation.
   */
  CGlobal();
  CGlobal(const CGlobal &);
  CGlobal & operator=(const CGlobal &);

 private:

  /**
   * The single instance of this class.
   */
  static CGlobal *pinstance;

  /**
   * The selected folder.
   */
  int m_cur_folder;

  /**
   * The selected message.
   */
  int m_cur_message;

  /**
   * The mode the client is in: index, maildir, or message.
   */
  std::string * m_mode;

  /**
   * The limit-string for the display of folders.
   */
  std::string * m_maildir_limit;

  /**
   * Currently selected folders.
   */
  std::vector < std::string > m_selected_folders;

  /**
   * The index-format string.
   */
  std::string * m_index_format;

  /**
   * The index-limit string.
   */
  std::string * m_index_limit;

  /**
   * The sent-mail path.
   */
  std::string * m_sent_mail;

  /**
   * The list of currently visible messages.
   */
  std::vector<CMessage*> *m_messages;

  /**
   * The settings we hold.
   */
  std::unordered_map<std::string, std::string *> m_variables;

};

#endif /* _global_h_ */
