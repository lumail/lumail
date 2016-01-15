/*
 * imap.h - Simple IMAP handling.
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

#include <stdio.h>
#include <stdlib.h>
#include <malloc.h>
#include <string.h>
#include <curl/curl.h>

#include <algorithm>
#include <sstream>
#include <vector>
#include <string>

#include "singleton.h"
#include "util.h"


/**
 *
 * Simple functions for IMAP operations:
 *
 * * Lists mailboxes.
 * * Count unread messages.
 * * Count total  messages - use "EXAMINE folder"
 *
 * TODO:
 *    Get a single message( folder, id )
 *
 */
class CIMAP : public Singleton<CIMAP>
{

public:

    /**
     * Constructor.
     */
    CIMAP();


    /**
     * Destructor.
     */
    ~CIMAP();


    /**
     * Setup login-credentials.
     */
    void set_password(std::string password);


    /**
     * Setup login-credentials.
     */
    void set_username(std::string username);


    /**
     * Setup the remote mail-server name, as an URL,
     * such as `imaps://mail.example.com/`
     */
    void set_server(std::string server);


    /**
     * Return a list of the folders available.
     */
    std::vector<std::string> getMaildirs();


    /**
     * Count the unread-messages in the specified folder.
     */
    int count_unread(std::string folder);


    /**
     * Count the total-messages in the specified folder.
     */
    int count_total(std::string folder);


    /**
     * This function is called whenever new data is received as a result
     * of an URL-fetch.
     *
     * We append the new data to the m_txt member-variable.
     */
    static size_t WriteMemoryCallback(void *ptr, size_t size, size_t nmemb, void *data);


private:

    /**
     * Username for IMAP login.
     */
    std::string m_username;

    /**
     * Password for IMAP login.
     */
    std::string m_password;

    /**
     * IMAP Server, as URL such as "imap://imap.example.com", or
     * "imaps://mail.example.com/"
     */
    std::string m_server;

    /**
     * Verify?
     */
    bool m_ssl_verify;


    /**
     * This text is appended to every time we make a request via libcurl.
     */
    std::string m_txt;
};
