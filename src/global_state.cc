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
#include <fstream>


#include "config.h"
#include "directory.h"
#include "file.h"
#include "global_state.h"
#include "history.h"
#include "imap_proxy.h"
#include "json/json.h"
#include "logfile.h"
#include "lua.h"
#include "maildir.h"
#include "message.h"
#include "util.h"

/*
 * Constructor
 */
CGlobalState::CGlobalState() : Observer(CConfig::instance())
{
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
        setenv("imap_username", config->get_string("imap.username").c_str(), 1);

        if ((config->get_string("imap.username", "") != "") &&
                (config->get_string("imap.password", "") != "") &&
                (config->get_string("imap.server", "") != ""))
            update_maildirs();
        else
        {
            CIMAPProxy *proxy = CIMAPProxy::instance();
            proxy->terminate();
        }
    }
    else if (key_name == "imap.password")
    {
        setenv("imap_password", config->get_string("imap.password").c_str(), 1);

        if ((config->get_string("imap.username", "") != "") &&
                (config->get_string("imap.password", "") != "") &&
                (config->get_string("imap.server", "") != ""))
            update_maildirs();
        else
        {
            CIMAPProxy *proxy = CIMAPProxy::instance();
            proxy->terminate();
        }
    }
    else if (key_name == "imap.server")
    {
        setenv("imap_server", config->get_string("imap.server").c_str(), 1);

        if ((config->get_string("imap.username", "") != "") &&
                (config->get_string("imap.password", "") != "") &&
                (config->get_string("imap.server", "") != ""))
            update_maildirs();
        else
        {
            CIMAPProxy *proxy = CIMAPProxy::instance();
            proxy->terminate();
        }
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
    return (m_maildirs);
}


/*
 * Update our cached maildir-list.
 */
void CGlobalState::update_maildirs()
{
    /*
     * If we have items already then remove them.
     */
    if (!m_maildirs.empty())
        m_maildirs.clear();


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
         * Read the output from our IMAP proxy.
         */
        CIMAPProxy *proxy = CIMAPProxy::instance();
        std::string json  = proxy->read_imap_output("list_folders\n");

        /*
         * Now parse the JSON into objects.
         */
        Json::Value root;
        Json::Reader reader;
        bool parsingSuccessful = reader.parse(json, root);

        if (!parsingSuccessful)
        {
            CLua *lua = CLua::instance();
            lua->on_error("Failed to parse JSON response to 'list_folders'.");

            config->set("maildir.max", 0);
            return;
        }

        Json::Value folders = root["folders"];

        int count  = 0;

        for (Json::ValueConstIterator it = folders.begin(); it != folders.end(); ++it)
        {
            /*
             * Get the values from the JSON array.
             */
            Json::Value single = (*it);
            int unread       = single["unread"].asInt();
            int total        = single["total"].asInt();
            std::string path = single["name"].asString();

            std::shared_ptr<CMaildir> m = std::shared_ptr<CMaildir>(new CMaildir(path, false));
            m->set_total(total);
            m->set_unread(unread);

            m_maildirs.push_back(m);

            count += 1;
        }

        config->set("maildir.max", count);
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

            m_maildirs.push_back(m);
        }
    }

    /*
     * Setup the size.
     */
    config->set("maildir.max", m_maildirs.size());
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
     * Get the currently selected maildir.
     */
    std::shared_ptr<CMaildir> current = current_maildir();

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
         * If we don't have a currently-selected folder then return.
         */
        if (! current)
            return;

        /*
         * Get the path of the currently selected folder.
         */
        std::string folder = current->path();

        /*
         * The server name is part of the cache.
         */
        CConfig *config = CConfig::instance();
        std::string imap_server = config->get_string("imap.server");
        std::string imap_cache  = config->get_string("imap.cache");

        if (imap_cache.empty())
            imap_cache = "/tmp";

        /*
         * Use our IMAP-proxy to get the message ID of each message
         * in the currently selected folder, as well as the flags of
         * the associated message.
         *
         * The retrival of the body will happen on-demand inside the
         * CMessage object.
         *
         */
        CIMAPProxy *proxy = CIMAPProxy::instance();
        std::string json  = proxy->read_imap_output("get_message_ids " + folder + "\n");

        int count = 0;

        /*
         * Now parse the JSON into objects.
         */
        Json::Value root;
        Json::Reader reader;
        bool parsingSuccessful = reader.parse(json, root);

        if (!parsingSuccessful)
        {
            CLua *lua = CLua::instance();
            lua->on_error("Failed to parse JSON response to 'get_messages'.");

            config->set("index.max", 0);
            return;
        }

        Json::Value messages = root["messages"];

        for (Json::ValueConstIterator it = messages.begin(); it != messages.end(); ++it)
        {
            /*
             * The array-member
             */
            Json::Value single = (*it);

            /*
             * The flags and ID of the message.
             */
            int id_val            = single["id"].asInt();
            std::string flags_val = single["flags"].asString();

            /*
             * Create a path to hold the IMAP message.
             *
             * The path will be $cache/$server/$folder/NN
             */
            std::string path = imap_cache;
            path += "/";
            path += escape_filename(imap_server);
            path += "/";
            path += escape_filename(folder);

            CDirectory::mkdir_p(path);

            path += "/";
            path += std::to_string(id_val);

            /*
             * Now create the message-object, pointing to the suitable
             * path, making sure that it is marked as non-local.
             */
            std::shared_ptr < CMessage > t = std::shared_ptr < CMessage >(new CMessage(path, false));
            t->path(path);

            /*
             * Split the flags into sane things.
             */
            std::string f;
            std::vector<std::string> flags = split(flags_val, ',');

            for (auto it = flags.begin() ; it != flags.end(); ++it)
            {
                std::string flag = (*it);

                if (flag == "\\Seen")
                    f += "S";

                if (flag == "\\Unseen")
                    f += "N";

                if (flag == "\\Answered")
                    f += "R";
            }

            /*
             * Empty flag == new message.
             */
            if (f.empty())
                f = "N";

            /*
             * Set the flags and ID to the message.  The flags will be
             * usable as-is.
             *
             * The ID means that the message-object can fetch its own
             * body on-demand when it wants to.
             */
            t->parent(current);
            t->set_imap_flags(f);
            t->set_imap_id(id_val);

            /*
             * Add the message to our list.
             */
            m_messages->push_back(t);

            count += 1;
        }

        config->set("index.max", count);
        return;
    }


    /*
     * Get the messages from the maildir.
     */
    if (current)
    {
        CMessageList contents = current->getMessages();

        for (std::shared_ptr<CMessage> content : contents)
        {
            m_messages->push_back(content) ;
        }
    }

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
 * Update the currently selected maildir, and trigger a refresh
 * of the message-cache.
 */
void CGlobalState::set_maildir(std::shared_ptr<CMaildir> updated)
{
    m_current_maildir = updated;

    update_messages();
}
