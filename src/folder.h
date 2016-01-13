/*
 * folder.h - Wrapper for a folder of messages.
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


/*
 * Only include this header one time.
 */
#pragma once


#include <string>
#include <vector>
#include <memory>


#include "message.h"


/**
 * This is an abstract class which represents a folder of messages.
 *
 * It is the base-class for CMaildir, and will be used to implement
 * CIMAPDir in the future.
 */
class CFolder
{
public:


    /**
     * Return the path we represent.
     */
    virtual std::string path() = 0;


    /**
     * The number of new messages for this maildir.
     */
    virtual int unread_messages() = 0;


    /**
     * The total number of messages for this maildir.
     */
    virtual int total_messages() = 0;


    /**
     * Get all of the messages in this maildir.
     */
    virtual CMessageList getMessages() = 0;


    /**
     * Save the given message in this maildir.
     */
    virtual bool saveMessage(std::shared_ptr <CMessage > msg) = 0;

};
