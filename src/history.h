/**
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
#include "utfstring.h"


/**
 * Singleton class to maintain a per-execution history of input
 * via the prompt.
 */
class CHistory
{

public:

    /**
     * Get access to the singleton instance.
     */
    static CHistory *Instance();

    /**
     * Return the size of the history.
     */
    int size();

    /**
     * Get the Nth piece of history.
     */
    UTFString at( size_t offset );

    /**
     * Return the first string which matches the given expression.
     */
    UTFString matching( UTFString input );

    /**
     * Add a new string to the history.
     */
    void add(UTFString entry);

    /**
     * Clear the history.
     */
    void clear();

    /**
     * Set the file to write history from.
     */
    void set_file( UTFString path );

protected:

    /**
     * Protected functions to allow our singleton implementation.
     */
    CHistory();
    CHistory(const CHistory &);
    CHistory & operator=(const CHistory &);

private:

    /**
     * The single instance of this class.
     */
    static CHistory *pinstance;

    /**
     * List of history items.
     */
    std::vector<UTFString> m_history;

    /**
     * The file to write to, may be unset.
     */
    UTFString m_filename;

};
