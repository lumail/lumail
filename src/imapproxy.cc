/*
 * imapproxy.cc - Manage our IMAP-proxy
 *
 * This file is part of lumail - http://lumail.org/
 *
 * Copyright (c) 2015 by Steve Kemp.  All rights reserved.
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


#include <cstdlib>
#include <fcntl.h>
#include <memory>
#include <signal.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/un.h>
#include <unistd.h>


#include "config.h"
#include "imapproxy.h"
#include "screen.h"


CIMAPProxy::CIMAPProxy()
{
    m_child = -1;
}


/*
 * Destructor
 */
CIMAPProxy::~CIMAPProxy()
{
    terminate();
}


/*
 * Terminate the child we've launched.
 */
void CIMAPProxy::terminate()
{
    if ( m_child != -1 )
    {
        kill(m_child, SIGKILL);
        m_child = -1;
    }

}



/*
 * Launch the child, if not already running.
 */
void CIMAPProxy::launch()
{
    size_t unused __attribute__((unused));

    if ( m_child == -1 )
    {

        CScreen *screen = CScreen::instance();
        screen->status_panel_append("Launching IMAP proxy... ");

        m_child = fork();
        if ( m_child == 0 )
        {
#if 0
            const char *cmd[]  = { "./perl.d/imapd", NULL };
            const char *env[5];

            CConfig *config = CConfig::instance();

            std::string i_u = std::string( "imap_username=" );
            i_u += config->get_string( "imap.username");

            std::string i_p = std::string( "imap_password=" );
            i_p += config->get_string( "imap.password");

            std::string i_s = std::string( "imap_server=" );
            i_s += config->get_string( "imap.server");

            std::string home = std::string("HOME=");
            home += getenv( "HOME" );

            env[0] = strdup( i_u.c_str());
            env[1] = strdup( i_p.c_str());
            env[2] = strdup( i_s.c_str());
            env[3] = strdup( home.c_str());
            env[4] = NULL;

            /*
             * Cause the child to go to /dev/null.
             */
            int fd = open("/dev/null", O_WRONLY);
            dup2(fd, 1);
            dup2(fd, 2);
#endif
            unused = execl( "./perl.d/imapd","imapd", NULL );
//            execve ("./perl.d/imapd", (char * const*)cmd, (char * const*)env);
        }

        sleep( 1.0 );
    }
}


/*
 * Read a string from our IMAP proxy, launching it first
 * if required.
 */
std::string CIMAPProxy::read_imap_output(std::string cmd)
{
    int sockfd;
    sockaddr_un addr;
    size_t unused __attribute__((unused));
    std::string result = "";

    /*
     * Launch the child.
     */
    launch();


    sockfd = socket(AF_UNIX, SOCK_STREAM, 0);
    addr.sun_family = AF_UNIX;

    /*
     * Use ~/.imap.sock as the path.
     */
    std::string path = getenv("HOME");
    path += "/.imap.sock";

    strcpy(addr.sun_path, path.c_str());

    if (connect(sockfd, (sockaddr*)&addr, sizeof(addr)) < 0)
    {
        return ("Connection failed!");
    }

    unused = write(sockfd, cmd.c_str(), cmd.length());

    char buf[65535];
    int rval;

    do
    {
        bzero(buf, sizeof(buf));

        if ((rval = read(sockfd, buf, sizeof(buf) - 1)) < 0)
        {
            // Failure
        }
        else if (rval == 0)
        {
            // End.
        }
        else
        {
            result += buf;
        }
    }
    while (rval > 0);

    close(sockfd);
    return (result);
}
