/*
 * lumail2.cc - Application entry-point.
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


#include <iostream>
#include <gmime/gmime.h>
#include <getopt.h>

#include "config.h"
#include "file.h"
#include "global_state.h"
#include "lua.h"
#include "maildir.h"
#include "message.h"
#include "message_part.h"
#include "screen.h"


/*
 * External flag for getopt - when set we can ignore unknown
 * argument errors.
 */
extern int opterr;



/*
 * The entry point to our code.
 */
int main(int argc, char *argv[])
{
    /*
     * Initiate mime.
     */
    g_mime_init(0);

    /*
     * Parse command-line arguments
     */
    int c;

    /*
     * Flags/things set by the command-line arguments.
     */
    std::vector < std::string > load;
    bool curses = true;


    /*
     * Default to loading some configuration files, if they
     * exist
     */
    if (CFile::exists("/etc/lumail2/lumail2.lua"))
        load.push_back("/etc/lumail2/lumail2.lua");

    /*
     * Get the home-directory of the current user.
     */
    std::string home;

    if (getenv("HOME") != NULL)
    {
        /*
         * If that worked then try to find things from beneath it.
         */
        home = getenv("HOME");

        if (!home.empty())
        {
            home = home + "/.lumail2/lumail2.lua";

            if (CFile::exists(home))
                load.push_back(home);
        }
    }


    while (1)
    {
        /* ignore unknown arguments. */
        opterr = 0;

        static struct option long_options[] =
        {
            {"no-curses", no_argument, 0, 'n'},
            {"load-file", required_argument, 0, 'l'},
            {"version", no_argument, 0, 'v'},
            {0, 0, 0, 0}
        };

        /* getopt_long stores the option index here. */
        int option_index = 0;

        c = getopt_long(argc, argv, "l:nv", long_options, &option_index);

        /* Detect the end of the options. */
        if (c == -1)
            break;

        switch (c)
        {
        case 'l':
            load.push_back(optarg);
            break;

        case 'n':
            curses = false;
            break;

        case 'v':
            std::cout << "Lumail2 " << LUMAIL_VERSION << std::endl;
            return 0;
            break;
        }
    }

    /*
     * Ensure that Lua has access to our command-line flags.
     */
    CLua  *instance = CLua::instance();
    instance->set_args(argv, argc);

    CScreen *screen = CScreen::instance();

    /*
     * If we're supposed to use curses then set it up.
     */
    if (curses == true)
        screen->setup();


    /*
     * This is annoying - we have to instantiate our singletons here.
     *
     * We MUST do this before we load any configuration files because
     * otherwise our listeners will not run, which means that things
     * like `global.history`, despite being defined in the lumail2.lua
     * file, will not get broadcast, and the setting will be worthless.
     *
     */
    {
        CGlobalState *global = CGlobalState::instance();
        global->update("there.is.no.match.here");

        /*
         * Launch time in seconds past the epoch.
         */
        CConfig *config = CConfig::instance();
        config->set("global.launched", time(NULL));
    }

    /*
     * Load any named script file(s) we're supposed to load.
     */
    if (!load.empty())
    {

        for (std::string filename : load)
        {
            if (CFile::exists(filename))
                instance->load_file(filename);
            else
            {
                screen->teardown();
                std::cerr << "File doesn't exist: " << filename << std::endl;
                return -1;
            }
        }
    }

    /*
     * Run the event-loop and terminate once that finishes.
     */
    if (curses == true)
    {
        screen->run_main_loop();
        screen->teardown();
    }

    /*
     * Close GMime.
     */
    g_mime_shutdown();

    return 0;
}
