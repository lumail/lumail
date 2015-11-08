/*
 * history.h - History wrapper for prompt-input
 *
 * This file is part of lumail: http://lumail.org/
 *
 * Copyright (c) 2013-2014 by Steve Kemp.  All rights reserved.
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
#include <vector>

#include "singleton.h"

/**
 * Singleton class to maintain a history of input which has been
 * entered into the prompt.
 */
class CHistory : public Singleton<CHistory>
{

public:

    /*
     * Return the size of the history.
     */
    int size();

    /*
     * Get the Nth piece of history.
     */
    std::string at(size_t offset);

    /*
     * Add a new string to the history.
     */
    void add(std::string entry);

    /*
     * Clear the history.
     */
    void clear();

    /*
     * Set the file to write history from.
     */
    void set_file(std::string path);

public:

    /*
     * Constructor.
     */
    CHistory();

private:

    /**
     * List of history items.
     */
    std::vector<std::string> m_history;

    /**
     * The file to write to, may be unset.
     */
    std::string m_filename;

};
