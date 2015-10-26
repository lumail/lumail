/**
 * $FILENAME - $TITLE
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


/**
 * Only include this header one time.
 */
#pragma once


#include <memory>
#include <string>
#include <vector>
#include "maildir.h"
#include "message.h"

/**
 * This is a class to hold "global state".
 *
 * In the future it will store all Maildirs, and the currently selected one.
 *
 * Similarly it will hold all the messages in the current directory as
 * well as the current one.
 *
 * For the moment it will only be a hacky-stub.
 */

class CGlobalState
{
private:
    CGlobalState();
    ~CGlobalState();

public:

    /**
     * Instance accessor - this object is a singleton.
     */
    static CGlobalState *instance();



    /**
     * Get all folders which match the current mode.
     */
    std::vector<std::shared_ptr<CMaildir> > get_maildirs();

    /**
     * Get all messages from the currently-selected folders.
     */
    std::vector<std::shared_ptr<CMessage> > *get_messages();

    /**
     * Get the currently selected message.
     */
    CMessage *current_message();

    /**
     * Called when a configuration-key has changed.
     */
    void config_key_changed(std::string name);

 private:
    void update_maildirs();
    void update_messages();

private:

    /**
     * All maildirs.
     */
    std::vector<CMaildir *> m_all_maildirs;

    /**
     * The current maildir.
     */
    CMaildir *m_current_maildir;

    /**
     * All messages.
     */
    std::vector<std::shared_ptr<CMessage> > *m_messages;

    /**
     * The list of all currently visible maildirs.
     */
    std::vector<std::shared_ptr<CMaildir> > *m_maildirs;

    /**
     * The currently selected message.
     */
    CMessage * m_current_message;
};
