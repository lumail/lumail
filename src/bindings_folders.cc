/**
 * bindings_folders.cc - Bindings for folder-related Lua primitives.
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
#include <fstream>
#include <stdlib.h>
#include <string.h>
#include <string>


#include "bindings.h"
#include "debug.h"
#include "file.h"
#include "global.h"
#include "lua.h"
#include "maildir.h"
#include "variables.h"


/**
 * Add the given folder to the selected set.
 */
int add_selected_folder(lua_State * L)
{
    /**
     * get the optional argument.
     */
    const char *str = lua_tostring(L, -1);

    CGlobal *global = CGlobal::Instance();
    CLua    *lua    = CLua::Instance();

    /**
     * The path that is being added.
     */
    std::string path;

    /**
     * default to the current folder.
     */
    if (str == NULL)
    {
        int selected = global->get_selected_folder();
        std::vector<CMaildir *> display = global->get_folders();

        if ( display.size()  == 0 )
            return 0;

        CMaildir *x = display[selected];
        path = x->path();
        global->add_folder(path.c_str());
    }
    else
    {
        path = std::string(str);
        global->add_folder(path);
    }

    global->set_selected_message(0);
    global->update_messages();
    global->set_message_offset(0);

    if ( ! path.empty() )
        lua->execute("on_folder_selection(\"" + path + "\");");

    return (0);
}


/**
 * Clear all currently selected folders.
 */
int clear_selected_folders(lua_State * L)
{
    CGlobal *global = CGlobal::Instance();
    global->unset_folders();
    global->set_selected_message(0);
    global->update_messages();
    global->set_message_offset(0);


    /**
     * Call our update with an empty path.
     */
    CLua *lua = CLua::Instance();
    lua->execute("on_folder_selection(\"\");");

    return 0;
}


/**
 * Get the currently selected folders.
 */
int selected_folders(lua_State * L)
{
    CGlobal *global = CGlobal::Instance();
    std::vector<std::string> selected = global->get_selected_folders();

    /**
     * Create the table.
     */
    lua_newtable(L);

    int i = 1;
    for (std::string folder : selected)
    {
        lua_pushnumber(L,i);
        lua_pushstring(L,folder.c_str());
        lua_settable(L,-3);
        i++;
    }

    return 1;
}


/**
 * Remove all currently selected folders.  Add single new one.
 */
int set_selected_folder(lua_State * L)
{
    /**
     * get the optional argument.
     */
    const char *str = lua_tostring(L, -1);

    CGlobal *global = CGlobal::Instance();
    global->unset_folders();

    /**
     * The path we're adding.
     */
    std::string path;

    /**
     * default to the current folder.
     */
    if (str == NULL)
    {
        std::vector<CMaildir *> display = global->get_folders();
        int selected = global->get_selected_folder();

        CMaildir *x = display[selected];
        path = x->path();
        global->add_folder(path.c_str());
    }
    else
    {
        path = std::string(str);
        global->add_folder(path.c_str());
    }

    global->update_messages();
    global->set_message_offset(0);

    if ( ! path.empty() )
    {
        CLua *lua = CLua::Instance();
        lua->execute("on_folder_selection(\"" + path + "\");");
    }

    return (0);
}


/**
 * Toggle the selection state of the currently selected folder.
 */
int toggle_selected_folder(lua_State * L)
{
    /**
     * get the optional argument.
     */
    const char *str = lua_tostring(L, -1);
    CGlobal *global = CGlobal::Instance();
    std::vector < std::string > sfolders = global->get_selected_folders();

    /**
     * default to the current folder.
     */
    std::string toggle;

    if (str == NULL)
    {
        std::vector<CMaildir *> display = global->get_folders();
        if ( display.size()  == 0 )
            return 0;

        int selected = global->get_selected_folder();
        CMaildir *x = display[selected];
        toggle = x->path();
    }
    else
    {
        toggle = std::string(str);
    }

    if (std::find(sfolders.begin(), sfolders.end(), toggle) != sfolders.end())
        global->remove_folder(toggle);
    else
        global->add_folder(toggle);

    global->update_messages();
    global->set_message_offset(0);

    if ( ! toggle.empty() )
    {
        CLua *lua = CLua::Instance();
        lua->execute("on_folder_selection(\"" + toggle + "\");");
    }
    return (0);
}



