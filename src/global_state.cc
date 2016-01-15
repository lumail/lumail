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

#include "config.h"
#include "file.h"
#include "global_state.h"
#include "history.h"
#include "lua.h"
#include "logfile.h"
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
    CConfig *config = CConfig::instance();

    /*
     * If we've changed global-mode then we might need to update
     * our messages/maildirs.
     */
    if (key_name == "global.mode")
    {
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
        std::string path = config->get_string("global.history");

        if (path.empty())
            return;

        CHistory *history = CHistory::instance();
        history->set_file(path);
    }
    else if (key_name == "global.logfile")
    {
        /*
         * The name of the logfile file.
         */
        std::string path = config->get_string("global.logfile");

        if (path.empty())
            return;

        CLogfile *log = CLogfile::instance();
        log->set_file(path);
    }
    else  if (key_name == "maildir.prefix")
    {
        /*
         * Otherwise if the maildir-prefix has changed update things.
         */
        update_maildirs();
    }
    else if (key_name == "imap.username")
    {
        CIMAP *imap = CIMAP::instance();
        std::string value = config->get_string("imap.username");
        imap->set_username(value);

        update_maildirs();
    }
    else if (key_name == "imap.password")
    {
        CIMAP *imap = CIMAP::instance();
        std::string value = config->get_string("imap.password");
        imap->set_password(value);

        update_maildirs();
    }
    else if (key_name == "imap.server")
    {
        CIMAP *imap = CIMAP::instance();
        std::string value = config->get_string("imap.server");
        imap->set_server(value);

        update_maildirs();
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
    return (*m_maildirs);
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
     *
     * If `imap.server`, `imap.user`, and `imap.password` are set
     * then retrieve the list of available folders via IMAP.
     *
     */
    CConfig *config = CConfig::instance();

    if ((config->get_string("imap.username", "") != "") &&
            (config->get_string("imap.password", "") != "") &&
            (config->get_string("imap.server", "") != ""))
    {
        /*
         * Get the IMAP handle.
         */
        CIMAP *x = CIMAP::instance();

        /*
         * For each IMAP folder create a corresponding CMaildir object.
         */
        std::vector<std::string> folders = x->getMaildirs();

        for (auto it = folders.begin() ; it != folders.end(); ++it)
        {
            /*
             * NOTE: We pass `false` to the constructor to mark it as non-local.
             */
            std::string path = (*it);
            std::shared_ptr<CMaildir> m = std::shared_ptr<CMaildir>(new CMaildir(path, false));

            m_maildirs->push_back(m);
        }

        config->set("maildir.max", m_maildirs->size());
        return;
    }


    /*
     * Get the maildir prefix - note that we allow an array to be used.
     */
    std::vector<std::string> prefixes = config->get_array("maildir.prefix");

    /*
     * If we've been given a string then we'll deal with that too.
     */
    if (prefixes.empty())
        prefixes.push_back(config->get_string("maildir.prefix"));


    /*
     * For each prefix add in the Maildirs.
     */
    for (auto it = prefixes.begin(); it != prefixes.end(); ++it)
    {
        std::string prefix = *it;

        /*
         * We'll store each maildir here.
         */
        std::vector<std::string> folders;
        folders = CFile::get_all_maildirs(prefix);

        /*
         * Construct the Maildir object.
         */
        for (std::string path : folders)
        {
            std::shared_ptr<CMaildir> m = std::shared_ptr<CMaildir>(new CMaildir(path));

            m_maildirs->push_back(m);
        }
    }

    /*
     * Setup the size.
     */
    config->set("maildir.max", m_maildirs->size());
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
        CMessageList contents = current->getMessages();

        for (std::shared_ptr<CMessage> content : contents)
        {
            m_messages->push_back(content) ;
        }
    }

    CConfig *config = CConfig::instance();
    config->set("index.max", m_messages->size());
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
