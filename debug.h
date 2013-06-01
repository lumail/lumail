/**
 * debug.h - Debug file for logging purposes.
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

#ifndef _debug_h_
#define _debug_h_ 1

#include <string>



/**
 * Simple macro to make logging as simple as:
 *
 *      DEBUG_LOG( "Some string" );
 *
 */
#ifndef DEBUG_LOG
#define DEBUG_LOG(x)                     \
    do {                                 \
        CDebug *d = CDebug::Instance();  \
        d->debug(x);                     \
    } while(0);
#endif



/**
 * Singleton class to maintain debug-log of execution.
 */
class CDebug
{

 public:

  /**
   * Get access to the singleton instance.
   */
  static CDebug *Instance();

  /**
   * Get a date/timestamp.
   */
  std::string timestamp();

  /**
   * Add a new string to the log.
   */
  void debug( std::string line );

 protected:

  /**
   * Protected functions to allow our singleton implementation.
   */
  CDebug();
  CDebug(const CDebug &);
  CDebug & operator=(const CDebug &);

 private:

  /**
   * The single instance of this class.
   */
  static CDebug *pinstance;

};

#endif	/* _debug_h_ */
