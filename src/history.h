/**
 * history.h - History wrapper for prompt-input
 *
 * This file is part of lumail: http://lumail.org/
 *
 * Copyright (c) 2013 by Steve Kemp.  All rights reserved.
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

#ifndef _history_h_
#define _history_h_ 1

#include <string>
#include <vector>

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
    std::string at( size_t offset );

    /**
     * Add a new string to the history.
     */
    void add( std::string entry);

    /**
     * Clear the history.
     */
    void clear();

    /**
     * Set the file to write history from.
     */
    void set_file( const char *path );

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
    std::vector<std::string> m_history;

    /**
     * The file to write to, may be unset.
     */
    std::string m_filename;

};

#endif /* _history_h_ */
