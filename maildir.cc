/**
 * maildir.cc - Utility functions for working with Maildirs
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

#include <vector>
#include <algorithm>
#include <sys/stat.h>
#include <sys/types.h>
#include <dirent.h>
#include <sys/types.h>

#include "global.h"
#include "maildir.h"
#include "message.h"

/**
 * Constructor.  NOP
 */
CMaildir::CMaildir(std::string path)
{
    m_path = path;
}

/**
 * Destructor.  NOP.
 */
CMaildir::~CMaildir()
{
}

/**
 * The number of new messages for this directory.
 */
int CMaildir::newMessages()
{
    return (CMaildir::countFiles(m_path + "/new"));
}

/**
 * The number of read messages for this directory.
 */
int CMaildir::availableMessages()
{
    return (CMaildir::countFiles(m_path + "/cur"));
}

/**
 * The friendly name of the maildir.
 */
std::string CMaildir::name()
{
    unsigned found = m_path.find_last_of("/");
    return (m_path.substr(found + 1));
}

/**
 * The full path to the folder.
 */
std::string CMaildir::path()
{
    return (m_path);
}

/**
 * Count files in a directory.
 */
int CMaildir::countFiles(std::string path)
{
    int count = 0;
    dirent *de;
    DIR *dp;

    dp = opendir(path.c_str());
    if (dp) {
	while (true) {
	    de = readdir(dp);
	    if (de == NULL)
		break;

	    if (!CMaildir::isDirectory(std::string(path + "/" + de->d_name)))
		count += 1;
	}
	closedir(dp);
    }
    return count;
}

/**
 * Return a sorted list of maildirs beneath the given path.
 */
std::vector < std::string > CMaildir::getFolders(std::string path)
{
    std::vector < std::string > result;
    dirent *de;
    DIR *dp;

    std::string prefix = path.empty()? "." : path.c_str();
    dp = opendir(prefix.c_str());
    if (dp) {
	while (true) {
	    de = readdir(dp);
	    if (de == NULL)
		break;

	    std::string subdir_name = std::string(de->d_name);
	    std::string subdir_path = std::string(prefix + "/" + subdir_name);
	    if (CMaildir::isMaildir(subdir_path))
		result.push_back(subdir_path);
	    else {
		if (subdir_name != "." && subdir_name != "..") {
		    DIR* sdp = opendir(subdir_path.c_str());
		    if (sdp) {
			closedir(sdp);
			std::vector < std::string > sub_maildirs;
			sub_maildirs = CMaildir::getFolders(subdir_path);
			std::vector < std::string >::iterator it;
			for (it = sub_maildirs.begin(); it != sub_maildirs.end(); ++it) {
			    result.push_back(*it);
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

/**
 * Get the messages in the folder.
 */
std::vector < CMessage > CMaildir::getMessages()
{
  /**
   * See what filter we have available.
   */
  CGlobal *global = CGlobal::Instance();
  std::string *filter = global->get_index_limit();

  std::vector < CMessage > result;
  dirent *de;
  DIR *dp;

  dp = opendir((m_path + "/cur").c_str());
  if (dp) {
    while (true) {
      de = readdir(dp);
      if (de == NULL)
        break;

      if (!CMaildir::isDirectory (std::string(m_path + "/cur/" + de->d_name)))
        {
          CMessage t = CMessage(std::string(m_path + "/cur/" + de->d_name));
          if ( strcmp(filter->c_str(), "all" ) == 0 )
            result.push_back(t);
          if ( ( strcmp(filter->c_str(), "new") == 0 ) && t.is_new() )
            result.push_back(t);
        }
    }
    closedir(dp);
  }

  dp = opendir((m_path + "/new").c_str());
  if (dp) {
    while (true) {
      de = readdir(dp);
      if (de == NULL)
        break;

      if (!CMaildir::isDirectory (std::string(m_path + "/new/" + de->d_name)))
      {
          CMessage t = CMessage(std::string(m_path + "/new/" + de->d_name));
          if ( strcmp(filter->c_str(), "all" ) == 0 )
            result.push_back(t);
          if ( ( strcmp(filter->c_str(), "new") == 0 ) && t.is_new() )
            result.push_back(t);
      }
    }
    closedir(dp);
  }
  return result;
}

/**
 * Is the given path a Maildir?
 */
bool CMaildir::isMaildir(std::string path)
{
    std::vector < std::string > dirs;
    dirs.push_back(path);
    dirs.push_back(path + "/cur");
    dirs.push_back(path + "/tmp");
    dirs.push_back(path + "/new");

    std::vector < std::string >::iterator it;
    for (it = dirs.begin(); it != dirs.end(); ++it) {
	if (!CMaildir::isDirectory(*it))
	    return false;
    }
    return true;
}

/**
 * Is the given path a directory?
 */
bool CMaildir::isDirectory(std::string path)
{
    struct stat sb;

    if (stat(path.c_str(), &sb) < 0)
	return 0;

    return (S_ISDIR(sb.st_mode));

}
