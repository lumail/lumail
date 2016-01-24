/*
 * imapproxy.h - Manage our IMAP-proxy
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

#include <string>

#include "singleton.h"

/**
 * The CImapProxy class is a singleton which is responsible for
 * launching our (perl) IMAP-proxy.
 *
 */
class CIMAPProxy : public Singleton<CIMAPProxy>
{
public:
    /**
     * Constructor.
     */
    CIMAPProxy();

    /**
     * Destructor - Kill our child-process, if it has been launched.
     */
    ~CIMAPProxy();

public:

    /**
     * Read a string from our IMAP proxy, launching it first
     * if required.
     */
    std::string read_imap_output(std::string cmd);

    /**
     * Launch an IMAP-proxy.
     */
    void launch();

    /**
     * Terminate the child we've launched.
     */
    void terminate();

private:
    /**
     * The handle to our child-process.
     */
    pid_t m_child;
};
