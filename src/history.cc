/*
 * history.cc - History wrapper for prompt-input
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

#include <assert.h>
#include <fstream>

#include "history.h"


/**
 * Get access to our singleton-object.
 */
CHistory *CHistory::instance()
{
    static CHistory *instance = new CHistory();
    return (instance);
}


/**
 * Constructor - This is private as this class is a singleton.
 */
CHistory::CHistory()
{
}


/**
 * Return the size of the history.
 */
int CHistory::size()
{
    return (m_history.size());
}


/**
 * Get the Nth piece of history.
 */
std::string CHistory::at(size_t offset)
{
    /**
     * Ensure the history offset is correctly bound.
     */
    assert(offset < m_history.size());

    return (m_history.at(offset));
}



/**
 * Add a new string to the history.
 */
void CHistory::add(std::string entry)
{
    /**
     * Don't add empty entries.
     */
    if (entry.empty())
        return;

    m_history.push_back(entry);
    assert(m_history.size() > 0);

    /**
     * If we've got a filename append the history.
     */
    if (! m_filename.empty())
    {
        std::fstream fs;
        fs.open(m_filename,  std::fstream::out | std::fstream::app);
        fs << entry << "\n";
        fs.close();
    }
}


/**
 * Clear the history.
 */
void CHistory::clear()
{
    m_history.clear();
    assert(m_history.size() == 0);
}

/**
 * Set the file.
 */
void CHistory::set_file(std::string filename)
{
    /**
     * Clear the current history.
     */
    m_history.clear();

    /**
     * Save the filename
     */
    m_filename = filename;

    /**
     * Load the prior history
     */
    std::ifstream input(m_filename);

    if (input.is_open())
    {
        while (input.good())
        {
            std::string line;
            getline(input, line);

            if (! line.empty())
                m_history.push_back(line);
        }

        input.close();
    }
}
