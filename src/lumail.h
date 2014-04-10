/**
 * lumail.h - The application itself.
 *
 * This file is part of lumail: http://lumail.org/
 *
 * Copyright (c) 2013-2014 by Steve Kemp.  All rights reserved.
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

#ifndef _lumail_h_
#define _lumail_h_ 1


/**
 * Forward declaration of classes.
 */
class CLua;
class CScreen;


#include <vector>


/**
 * This class is the driver for our whole application.
 */
class CLumail
{

public:

    /**
     * Constructor:  Setup the screen, Gmime, etc.
     */
    CLumail();

    /**
     * Destructor.  Tear down the screen, etc.
     */
    ~CLumail();

    /**
     * Load our standard init files, also the extra files if they're present.
     */
    bool load_init_files( std::vector<std::string> extra );

    /**
     * Open the given maildir.
     */
    bool open_folder( std::string path );

   /**
    * Can the socket be read from?  Timeout after N-milliseconds.
    */
    bool can_read(int socket, int n );

    /**
     * Process input from the domain socket.
     */
    void domain_socket_pump(int socket);

    /**
     * Draw/Refresh the display and intepret keys.
     */
    void run_event_loop();

private:

    /**
     * Handle to our lua wrapper.
     */
    CLua *m_lua;

    /**
     * The screen object.
     */
    CScreen *m_screen;

};


#endif /* _lumail_h_ */
