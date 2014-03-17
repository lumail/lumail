/**
 * lumail.cc - The application itself.
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



#include <algorithm>
#include <curses.h>
#include <getopt.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/un.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <signal.h>
#include <poll.h>
#include <string.h>

#include "debug.h"
#include "file.h"
#include "global.h"
#include "input.h"
#include "lua.h"
#include "lumail.h"
#include "maildir.h"
#include "message.h"
#include "screen.h"
#include "version.h"




/**
 * Constructor:  Setup the screen, Gmime, etc.
 */
CLumail::CLumail()
{
    if ( getenv( "RFC2047" ) != NULL)
        g_mime_init(GMIME_ENABLE_RFC2047_WORKAROUNDS);
    else
        g_mime_init (0);

    m_screen = new CScreen();
    m_screen->setup();

    /**
     * Get a lua instance.
     */
    m_lua = CLua::Instance();
}



/**
 * Destructor.  Tear down the screen, etc.
 */
CLumail::~CLumail()
{
    endwin();

    g_mime_shutdown();
}


/**
 * Load our standard init files, also the named file if it is present.
 */
bool CLumail::load_init_files( std::vector<std::string> extra )
{
    /**
     * Number of init-files we've loaded.
     */
    int init = 0;

    /**
     * If we've got /etc/lumail.lua then load it.
     */
    if ( m_lua->load_file("/etc/lumail.lua") )
    {
        init += 1;
    }

    /**
     * Load /etc/lumail.d/ if present.
     */
    if ( CFile::is_directory( "/etc/lumail.d/" ) )
    {
        std::vector<std::string> files = CFile::files_in_directory( "/etc/lumail.d" );

        for (std::string file : files)
        {
            /**
             * If the file ends in .lua then load it.
             */
            size_t offset = file.rfind('.');
            if(offset != std::string::npos)
            {
                /**
                 * Get the lower-case version.
                 */
                std::string extension = file.substr(offset+1);
                std::transform(extension.begin(), extension.end(),
                               extension.begin(), tolower );

                if ( extension == "lua" )
                {
                    m_lua->load_file( file );
                    init += 1;
                }
            }
        }
    }


    /**
     * Load the init-file from the users home-directory, if we can.
     */
    std::string home = getenv( "HOME" );
    if ( ( ! home.empty() ) && ( CFile::exists( home + "/.lumail/config.lua" ) ) )
         if ( m_lua->load_file( home + "/.lumail/config.lua") )
             init += 1;

    /**
     * If we have any init files specified then load it up too.
     */
    for (std::string path : extra)
    {
        if (CFile::exists( path ))
        {
            if ( m_lua->load_file(path.c_str()) )
                init += 1;
        }
    }

    if ( init > 0 )
    {
        /**
         * We're starting, so call the on_start() function.
         */
        m_lua->execute("on_start()");

        return true;
    }

    return false;
}


/**
 * Open the given maildir.
 */
bool CLumail::open_folder(std::string folder)
{

    /**
     * Remove any trailing "/" character(s).
     */
    while ( folder.at(folder.size()-1) == '/' )
        folder = folder.substr(0,folder.size()-1);

    if ( CMaildir::is_maildir( folder ) )
    {
        /**
         * Open the folder.
         */
        m_lua->execute( "set_selected_folder( \"" + folder + "\" );" );
        m_lua->execute( "global_mode( \"index\" );" );

        return true;
    }
    else
    {
        return false;
    }
}

/**
 * Can the socket be read from?  Timeout after N-milliseconds.
 */
bool CLumail::can_read(int socket, int n )
{

#ifdef DOMAIN_SOCKET

    int r = 0;
    struct pollfd fds[1];

    fds[0].fd = socket;
    fds[0].events = POLLIN;
    do {
        r = poll(fds, 1, n);
    } while (r == -1 && errno == EINTR);
    return (r > 0);
#else
    return false;
#endif

}


/**
 * Process input from the domain socket.
 */
void CLumail::domain_socket_pump(int socket_fd)
{
#ifdef DOMAIN_SOCKET

    char buf[1024];
    int rval;

    /**
     * If we can't read input then return after 50ms.
     */
    if ( ! can_read( socket_fd , 50 ) )
        return;

    /**
     * Look for a new connection.
     */
    int msgsock = accept(socket_fd, 0, 0);
    if (msgsock == -1)
    {
#ifdef LUMAIL_DEBUG
        DEBUG_LOG( "Error accepting new domain-socket connection" );
#endif
        return;
    }

    /**
     * Process input.
     */
    do {
        bzero(buf, sizeof(buf));
        if ((rval = read(msgsock, buf, 1024)) < 0)
        {
#ifdef LUMAIL_DEBUG
        DEBUG_LOG( "Error reading from domain-socket" );
#endif
        }
        else if (rval == 0)
        {
#ifdef LUMAIL_DEBUG
        DEBUG_LOG( "Disconnection from domain-socket" );
#endif
        }
        else
        {
#ifdef LUMAIL_DEBUG
            DEBUG_LOG( "Read from domain-socket:"  + std::string(buf) );
#endif
            m_lua->execute(buf);
        }
    } while (rval > 0);

#endif

}

/**
 * Draw/Refresh the display and intepret keys.
 */
void CLumail::run_event_loop()
{
    /**
     * Now enter our event-loop
     */
    while (true)
    {

        /**
         * Do we have a domain-socket?
         */
        CGlobal *global = CGlobal::Instance();
        int sock = global->get_domain_socket();
        if ( sock >= 0 )
                domain_socket_pump( sock );


        gunichar key;
        int  r = CInput::Instance()->get_wchar(&key);

        if (r== ERR)
        {
            /*
             * Timeout - so we go round the loop again.
             */
            m_lua->execute("on_idle()");
        }
        else
        {
            /**
             * The human-readable version of the key which has
             * been pressed.
             *
             * i.e. Ctrl-r -> ^R.
             */
            const char *name = CScreen::get_key_name( key, r == KEY_CODE_YES );

            /**
             * See if we can handle it via our keyboard map, or
             * the Lua function "on_key".
             */
            if ( (!m_lua->on_key( name )) && ( !m_lua->on_keypress(name)) )
            {
                /**
                 * Both calls failed, so show a message.
                 */
                std::string foo = "msg(\"Unbound key: ";
                foo += std::string(name) + "\");";
                m_lua->execute(foo);
            }
        }


        m_screen->refresh_display();
    }

}

