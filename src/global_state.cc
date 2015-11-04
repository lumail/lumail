/*
 * global_state.cc - Maintain shared-state.
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
#include <regex>

#include "config.h"
#include "file.h"
#include "global_state.h"
#include "history.h"
#include "lua.h"
#include "maildir.h"
#include "message.h"


/**
 * Accessor for our singleton.
 */
CGlobalState * CGlobalState::instance()
{
    static CGlobalState *instance = new CGlobalState();
    return (instance);
}


/**
 * Constructor
 */
CGlobalState::CGlobalState()
{
    m_maildirs = NULL;
    m_messages = NULL;
}


/**
 * Destructor
 */
CGlobalState::~CGlobalState()
{
}


/**
 * Get the currently selected message.
 */
std::shared_ptr<CMessage>CGlobalState::current_message()
{
    return (m_current_message);
}

/**
 * Change the currently selected message.
 */
void CGlobalState::set_message(std::shared_ptr<CMessage> update)
{
    m_current_message = update;
}


/**
 * Called when a configuration-key has changed.
 */
void CGlobalState::config_key_changed(std::string name)
{
    /**
     * If we've changed global-mode then we might need to update
     * our messages/maildirs.
     */
    if (name == "global.mode")
    {
        CConfig *config = CConfig::instance();
        std::string new_mode = config->get_string("global.mode");

        /**
         * Update our cached list of messages in this maildir.
         */
        if (! new_mode.empty() && (new_mode == "index"))
            update_messages();

        /**
         * Update the list of maildirs.
         */
        if (!new_mode.empty() && (new_mode == "maildir"))
            update_maildirs();

        /**
         * Reset the horizontal scroll to be zero.
         */
        config->set("global.horizontal", "0", false);
    }

    /**
     * The name of the history file.
     */
    if (name == "global.history")
    {
        CConfig *config = CConfig::instance();
        std::string path = config->get_string("global.history");

        if (path.empty())
            return;

        CHistory *history = CHistory::instance();
        history->set_file(path);
    }

    /**
     * Otherwise if the maildir-prefix or limit has changed update things
     */
    if ((name == "maildir.limit") || (name == "maildir.prefix"))
    {
        update_maildirs();
    }

    /**
     * If the index-limit has changed update that too.
     */
    if (name == "index.limit")
    {
        update_messages();
    }

    /**
     * Get access to our Lua magic.
     */
    CLua *lua = CLua::instance();
    lua_State * l = lua->state();

    /**
     * If there is a Config:key_changed() function, then call it.
     */
    lua_getglobal(l, "Config");
    lua_getfield(l, -1, "key_changed");

    if (lua_isnil(l, -1))
        return;

    /**
     * Call the function.
     */
    lua_pushstring(l, name.c_str());
    lua_pcall(l, 1, 0, 0);
}


/**
 * Get all messages from the currently selected folder.
 */
CMessageList * CGlobalState::get_messages()
{
    return (m_messages);
}

/**
 * Get the maildirs which are visible.
 */
std::vector<std::shared_ptr<CMaildir> > CGlobalState::get_maildirs()
{
    CMaildirList display;

    /**
     * If we have no folders then we must return the empty set.
     *
     * Most likely cause?  The maildir_prefix isn't set, or is set incorrectly.
     */
    if (m_maildirs == NULL)
        return (display);

    /**
     * Filter the folders to those we can display
     */
    for (std::shared_ptr<CMaildir> maildir : (*m_maildirs))
    {
        display.push_back(maildir);
    }

    return (display);
}

/**
 * Update our cached maildir-list.
 */
void CGlobalState::update_maildirs()
{
    /**
     * If we have items already then free each of them.
     */
    if (m_maildirs != NULL)
    {
        delete(m_maildirs);
        m_maildirs = NULL;
    }

    m_maildirs = new CMaildirList;

    /**
     * Get the maildir prefix.
     */
    CConfig *config = CConfig::instance();
    std::string prefix = config->get_string("maildir.prefix");

    if (prefix.empty())
        return;


    /**
     * Get the maildir.limit.
     */
    std::string limit = config->get_string("maildir.limit");

    if (limit.empty())
        limit = "all";


    /**
     * We'll store each maildir here.
     */
    std::vector<std::string> folders;
    folders = CFile::get_all_maildirs(prefix);

    for (std::string path : folders)
    {
        std::shared_ptr<CMaildir> m = std::shared_ptr<CMaildir>(new CMaildir(path));

        if (limit == "all")
            m_maildirs->push_back(m);
        else  if (limit == "new")
        {
            if (m->unread_messages() > 0)
                m_maildirs->push_back(m);
        }
        else
        {
            if (std::regex_match(path, std::regex(limit)))
                m_maildirs->push_back(m);
        }
    }

    /**
     * Setup the size.
     */
    config->set("maildir.max", std::to_string(m_maildirs->size()));
}

void CGlobalState::update_messages()
{
    /**
     * If we have items already then free each of them.
     */
    if (m_messages != NULL)
        delete(m_messages);

    /**
     * create a new store.
     */
    m_messages = new CMessageList;

    /**
     * Get the selected maildirs.  If any.
     */
    std::shared_ptr<CMaildir> current = current_maildir();

    if (current)
    {
        CConfig *config = CConfig::instance();
        std::string limit = config->get_string("index.limit");

        if (limit.empty())
            limit = "all";

        CMessageList contents = current->getMessages();

        /**
         * Append to the list of messages combined.
         */
        for (std::shared_ptr<CMessage> content : contents)
        {
            if (limit == "all")
                m_messages->push_back(content) ;

            if (limit == "new")
                if (content->is_new())
                    m_messages->push_back(content);
        }
    }

    CConfig *config = CConfig::instance();
    config->set("index.max", std::to_string(m_messages->size()));
}


std::shared_ptr<CMaildir>CGlobalState::current_maildir()
{
    return (m_current_maildir);
}
void CGlobalState::set_maildir(std::shared_ptr<CMaildir> updated)
{
    m_current_maildir = updated;
}
