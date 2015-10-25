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

#include "config.h"
#include "global_state.h"


/**
 * Accessor for our singleton.
 */
CConfig * CConfig::instance()
{
    static CConfig *instance = new CConfig();
    return (instance);
}


/**
 * Constructor
 */
CConfig::CConfig()
{
}


/**
 * Destructor
 */
CConfig::~CConfig()
{
    /**
     * Free all our entries.
     */
    for (std::vector < CConfigEntry * >::iterator it = m_entries.begin();
            it != m_entries.end(); ++it)
    {
        CConfigEntry *tmp = (*it);

        if (tmp->type == CONFIG_STRING)
        {
            delete(tmp->value.str);
        }
        else if (tmp->type == CONFIG_ARRAY)
        {
            delete(tmp->value.array);
        }
        else
        {
            throw "Unknown config-type!";
        }

        free(tmp);
    }
}


/**
 * Delete the value of the named key.
 */
void CConfig::delete_key(std::string name)
{
    int i = 0;
    int found = -1;

    for (std::vector < CConfigEntry * >::iterator it = m_entries.begin();
            it != m_entries.end(); ++it)
    {
        if (*(*it)->name == name)
            found = i;

        i += 1;
    }

    /**
     * If we found the entry then we can remove it.
     */
    if (found != -1)
        m_entries.erase(m_entries.begin() + found);
}


/**
 * Get a configuration-value by name, returning NULL on failure.
 */
CConfigEntry * CConfig::get(std::string name)
{

    for (std::vector < CConfigEntry * >::iterator it = m_entries.begin();
            it != m_entries.end(); ++it)
    {
        CConfigEntry *tmp = (*it);

        if (*tmp->name == name)
            return (tmp);
    }

    return NULL;
}


/**
 * Get all the keys we know about.
 */
std::vector < std::string > CConfig::keys()
{
    std::vector < std::string > results;

    for (std::vector < CConfigEntry * >::iterator it = m_entries.begin();
            it != m_entries.end(); ++it)
    {
        CConfigEntry *
        tmp = (*it);
        results.push_back(*tmp->name);
    }

    return (results);
}


/**
 * Set the given key to the single string-value.
 *
 * This replaces any prior value which might have been stored under that key.
 */
void CConfig::set(std::string name, std::string val)
{
    /**
     * Delete the existing value(s).
     */
    delete_key(name);

    /**
     * Create the new the new entry.
     */
    CConfigEntry *x = (CConfigEntry *) malloc(sizeof(CConfigEntry));

    if (x == NULL)
        throw "Memory allocation failure";

    /**
     * Store the data
     */
    x->name = new std::string(name);
    x->type = CONFIG_STRING;
    x->value.str = new std::string(val);

    /**
     * Store the entry.
     */
    m_entries.push_back(x);

    /**
     * Notify our global state of the variable change.
     */
    CGlobalState *global = CGlobalState::instance();
    global->config_key_changed(name);
}


/**
 * Set the given key to the array of strings.
 *
 * This replaces any prior value which might have been stored under that key.
 */
void CConfig::set(std::string name, std::vector < std::string > entries)
{
    /**
     * Delete the existing value(s).
     */
    delete_key(name);

    /**
     * Create the new entry.
     */
    CConfigEntry *x = (CConfigEntry *) malloc(sizeof(CConfigEntry));

    /**
     * Store the data.
     */
    x->name = new std::string(name);
    x->type = CONFIG_ARRAY;
    x->value.array = new std::vector < std::string >;

    /**
     * Copy the string-values.
     */
    for (std::vector < std::string >::iterator it = entries.begin();
            it != entries.end(); ++it)
    {
        x->value.array->push_back((*it));
    }

    /**
     * Add the entry.
     */
    m_entries.push_back(x);

    /**
     * Notify our global state of the variable change.
     */
    CGlobalState *global = CGlobalState::instance();
    global->config_key_changed(name);
}
