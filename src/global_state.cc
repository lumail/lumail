/*
 * global_state.cc - Access our shared/global message/maildir state.
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


/*
 * Constructor
 */
CGlobalState::CGlobalState() : Observer(CConfig::instance())
{
    m_maildirs = NULL;
    m_messages = NULL;
    m_current_message = NULL;
    update_messages();
    update_maildirs();

}


/*
 * Destructor
 */
CGlobalState::~CGlobalState()
{
}


/*
 * Get the currently selected message.
 */
std::shared_ptr<CMessage> CGlobalState::current_message()
{
    return (m_current_message);
}


/*
 * Change the currently selected message.
 */
void CGlobalState::set_message(std::shared_ptr<CMessage> update)
{
    m_current_message = update;
}


/*
 * This method is called when a configuration key changes,
 * via our observer implementation.
 */
void CGlobalState::update(std::string key_name)
{
    /*
     * If we've changed global-mode then we might need to update
     * our messages/maildirs.
     */
    if (key_name == "global.mode")
    {
        CConfig *config = CConfig::instance();
        std::string new_mode = config->get_string("global.mode");

        /*
         * Update our cached list of messages in this maildir.
         */
        if (! new_mode.empty() && (new_mode == "index"))
            update_messages();

        /*
         * Update the list of maildirs.
         */
        if (!new_mode.empty() && (new_mode == "maildir"))
            update_maildirs();

        /*
         * Reset the horizontal scroll to be zero.
         */
        config->set("global.horizontal", 0, false);
    }
    else if (key_name == "global.history")
    {
      /*
       * The name of the history file.
       */
        CConfig *config = CConfig::instance();
        std::string path = config->get_string("global.history");

        if (path.empty())
            return;

        CHistory *history = CHistory::instance();
        history->set_file(path);
    }
    else  if ((key_name == "maildir.limit") || (key_name == "maildir.prefix"))
    {
      /*
       * Otherwise if the maildir-prefix or limit has changed update things
       */
        update_maildirs();
    }
    else if (key_name == "index.limit")
    {
      /*
       * If the index-limit has changed update that too.
       */
      update_messages();
    }
}


/*
 * Get the messages from the currently selected folder.
 */
CMessageList * CGlobalState::get_messages()
{
    return (m_messages);
}


/*
 * Get the available maildirs.
 */
std::vector<std::shared_ptr<CMaildir>> CGlobalState::get_maildirs()
{
  return( *m_maildirs );
}


/*
 * Update our cached maildir-list.
 */
void CGlobalState::update_maildirs()
{
    /*
     * If we have items already then free each of them.
     */
    if (m_maildirs != NULL)
    {
        delete(m_maildirs);
        m_maildirs = NULL;
    }

    m_maildirs = new CMaildirList;

    /*
     * Get the maildir prefix.
     */
    CConfig *config = CConfig::instance();
    std::string prefix = config->get_string("maildir.prefix");

    if (prefix.empty())
        return;


    /*
     * Get the maildir.limit.
     */
    std::string limit = config->get_string("maildir.limit");

    if (limit.empty())
        limit = "all";


    /*
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

    /*
     * Setup the size.
     */
    config->set("maildir.max", std::to_string(m_maildirs->size()));
}


/*
 * Update the cached list of messages.
 */
void CGlobalState::update_messages()
{
    /*
     * If we have items already then free each of them.
     */
    if (m_messages != NULL)
        delete(m_messages);

    /*
     * create a new store.
     */
    m_messages = new CMessageList;

    /*
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

        /*
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


/*
 * Return the currently-selected maildir.
 */
std::shared_ptr<CMaildir>CGlobalState::current_maildir()
{
    return (m_current_maildir);
}


/*
 * Update the currently selected maildir.
 */
void CGlobalState::set_maildir(std::shared_ptr<CMaildir> updated)
{
    m_current_maildir = updated;
}
