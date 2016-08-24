/*
 * global_state.h - Access our shared/global message/maildir state.
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


#pragma once


#include <memory>
#include <string>
#include <vector>

#include "maildir.h"
#include "message.h"
#include "observer.h"
#include "singleton.h"


/**
 * This is a class to hold "global state", which primarily means that
 * it stores the lists of current maildirs, messages, as well as the
 * single selected message.
 *
 * The class itself is both a singleton and an observer - observing
 * changes in the `CConfig` global singleton.
 */

class CGlobalState : public Singleton<CGlobalState>, public Observer
{
public:
    /**
     * Constructor.
     */
    CGlobalState();

    /**
     * Destructor.
     */
    ~CGlobalState();

public:

    /**
     * Get the available maildirs.
     */
    std::vector<std::shared_ptr<CMaildir>> get_maildirs();

    /**
     * Get the messages in the currently-selected folder.
     */
    std::vector<std::shared_ptr<CMessage> > *get_messages();

    /**
     * Get the currently selected message.
     */
    std::shared_ptr<CMessage> current_message();

    /**
     * Select the specified message.
     */
    void set_message(std::shared_ptr<CMessage> new_cur);

    /**
     * Get the currently selected maildir.
     */
    std::shared_ptr<CMaildir > current_maildir();

    /**
     * Update the currently selected maildir, and trigger a refresh
     * of the message-cache.
     */
    void set_maildir(std::shared_ptr<CMaildir >  folder);

public:

    /**
     * Update the list of cached maildirs.
     */
    void update_maildirs();

    /**
     * Update our cache of messages, that cached list is returned
     * via `get_messages`.
     */
    void update_messages(bool force = false);

    /**
     * This method is called when a configuration key changes,
     * via our observer implementation.
     */
    void update(std::string key_name);

private:

    /**
     * The list of all currently visible maildirs.
     */
    std::vector<std::shared_ptr<CMaildir> > m_maildirs;

    /**
     * The currently selected maildir.
     */
    std::shared_ptr<CMaildir> m_current_maildir;

    /**
     * All messages.
     */
    std::vector<std::shared_ptr<CMessage> > *m_messages;

    /**
     * The currently selected message.
     */
    std::shared_ptr<CMessage> m_current_message;
};
