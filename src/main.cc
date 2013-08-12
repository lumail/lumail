/**
 * main.cc - Driver/Wrapper for our lumail script.
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
#include <cstdlib>
#include <curses.h>
#include <iostream>
#include <fstream>
#include <getopt.h>
#include <glib.h>
#include <glib/gstdio.h>
#include <gmime/gmime.h>

#include "debug.h"
#include "lumail.h"
#include "version.h"



/**
 * Entry point to our code.
 */
int main(int argc, char *argv[])
{
    /**
     * Parse command-line arguments
     */
    int c;

    bool version         = false; /* show version */
    bool exit_after_eval = false; /* exit after eval? */
    std::string rcfile   = "";    /* load rc file */
    std::string eval     = "";    /* code to evaluate */
    std::string folder   = "";    /* open folder */
    std::string debug    = "";    /* debug-log */

    while (1)
    {
        static struct option long_options[] =
            {
                {"debug", required_argument, 0, 'd'},
                {"eval", required_argument, 0, 'e'},
                {"exit", no_argument, 0, 'x'},
                {"folder", required_argument, 0, 'f'},
                {"rcfile", required_argument, 0, 'r'},
                {"version", no_argument, 0, 'v'},
                {0, 0, 0, 0}
            };

        /* getopt_long stores the option index here. */
        int option_index = 0;

        c = getopt_long(argc, argv, "vr:f:e:", long_options, &option_index);

        /* Detect the end of the options. */
        if (c == -1)
            break;

        switch (c)
        {
        case 'd':
#ifdef LUMAIL_DEBUG
            debug = optarg;
#else
            std::cout << "Debug support was not compiled in." << std::endl;
            exit(1);
#endif
            break;
        case 'e':
            eval = optarg;
            break;
        case 'f':
            folder = optarg;
            break;
        case 'r':
            rcfile = optarg;
            break;
        case 'v':
            version = true;
            break;
        case 'x':
            exit_after_eval = true;
            break;
        case '?':
            /* getopt_long already printed an error message. */
            exit(1);
            break;

        default:
            std::cerr << "Unknown argument" << std::endl;
            exit (1);
        }
    }

    if (version)
    {
        std::cout << "lumail v" << LUMAIL_VERSION ;
#ifdef LUMAIL_DEBUG
        std::cout << " (debug-build)";
#endif
        std::cout << std::endl;


        std::cout << "Built against " << LUA_VERSION;

        char g_ver[1024] = { '\0' };
        snprintf(g_ver, sizeof(g_ver)-1, " and GMime %d.%d.%d",
                 gmime_major_version, gmime_minor_version, gmime_micro_version );
        std::cout << g_ver ;

        std::cout << std::endl;
        return 0;
    }


    /**
     * Set the debug-logfile name.
     */
    if ( !debug.empty() )
    {
        CDebug *d = CDebug::Instance();
        d->set_logfile( debug );
    }


    /**
     * Create the application.
     */
    CLumail *obj = new CLumail();

    /**
     * Load the default init files, and optionally the
     * one specified on the command line.
     */
    if ( !obj->load_init_files( rcfile ) )
    {
        delete( obj );

        std::cerr << "No init file was loaded!" << std::endl;

        std::cerr << "We tried to load both: /etc/lumail.lua & ~/.lumail/config.lua."
                  << std::endl;

        std::cerr << "You should specify an init file to load via --rcfile"
                  << std::endl;
        return -1;
    }


    /**
     * If we have a starting folder, select it.
     */
    if ( !folder.empty() )
    {
        if ( ! obj->open_folder( folder ) )
        {
            CLua *lua = CLua::Instance();
            lua->execute("msg(\"Startup folder is not a Maildir!\");" );
        }
    }

    /**
     * If evaluating code then do that now, then exit.
     */
    if ( !eval.empty() )
    {
        CLua *lua = CLua::Instance();
        lua->execute( eval.c_str() );

        if ( exit_after_eval )
        {
            lua->execute( "exit()" );
        }
    }


    /**
     * Now enter our event-loop
     */
    obj->run_event_loop();


    /**
     * Cleanup.
     */
    delete(obj);
    return -3;
}
