/*
 * config.cc - A configuration-value holding class
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

#include <algorithm>
#include "stdlib.h"

#include "config.h"



/*
 * The constructor for the singleton CConfig class.
 *
 * The constructor sets up some default configuration values,
 * which will be useful to the lua-environment we run.
 *
 */
CConfig::CConfig()
{
    set("global.mode", "maildir", false);

    set("index.limit", "all", false);
    set("index.max", "0", false);

    set("maildir.limit", "all", false);
    set("maildir.max", "0", false);

    set("global.mailer", "/usr/lib/sendmail -t", false);
#ifdef LUMAIL_VERSION
    set("global.version", LUMAIL_VERSION, false);
#endif
    set("global.timeout", 200, false);
}


/*
 * Destructor
 */
CConfig::~CConfig()
{
}


/*
 * Delete all keys and their associated values.
 */
void CConfig::remove_all()
{
    /*
     * Get all our known configuration-keys.
     */
    std::vector<std::string> existing = keys();

    /*
     * For each key, remove it and the value.
     */
    for (auto it = existing.begin(); it != existing.end(); ++it)
        delete_key(*it);
}


/*
 * Remove the value of the given key.
 */
void CConfig::delete_key(std::string name)
{
    CConfigEntry *tmp = m_entries[name];

    if (! tmp)
        return;

    if (tmp->type == CONFIG_STRING)
        delete(tmp->value.str);
    else if (tmp->type == CONFIG_INTEGER)
        delete(tmp->value.value);
    else if (tmp->type == CONFIG_ARRAY)
        delete(tmp->value.array);
    else
        throw "Unknown config-type!";

    free(tmp);

    m_entries[name] = NULL;
}


/*
 * Get a configuration-value by name, returning NULL on failure.
 */
CConfigEntry * CConfig::get(std::string name)
{
    return (m_entries[ name ]);
}


/*
 * Get all the keys we know about.
 */
std::vector < std::string > CConfig::keys()
{
    std::vector < std::string > results;

    for (auto it = m_entries.begin(); it != m_entries.end(); ++it)
    {
        CConfigEntry *tmp = it->second;

        if (tmp)
            results.push_back(*tmp->name);
    }

    std::sort(results.begin(), results.end());
    return (results);
}


/*
 * Set the given key to the single string-value.
 *
 * This replaces any prior value which might have been stored under that key.
 */
void CConfig::set(std::string name, std::string val, bool notify)
{
    /*
     * Delete any existing value stored under this key.
     */
    delete_key(name);

    /*
     * Create the new the new entry.
     */
    CConfigEntry *x = (CConfigEntry *) malloc(sizeof(CConfigEntry));

    if (x == NULL)
        throw "Memory allocation failure";

    /*
     * Store the data
     */
    x->name = new std::string(name);
    x->type = CONFIG_STRING;
    x->value.str = new std::string(val);

    /*
     * Store the entry.
     */
    m_entries[name] = x;

    /*
     * Notify our global state of the variable change.
     */
    if (notify)
        notify_watchers(name);
}



/*
 * Set the given key to the single int-value.
 *
 * This replaces any prior value which might have been stored under that key.
 */
void CConfig::set(std::string name, int val, bool notify)
{
    /*
     * Delete any existing value stored under this key.
     */
    delete_key(name);

    /*
     * Create the new the new entry.
     */
    CConfigEntry *x = (CConfigEntry *) malloc(sizeof(CConfigEntry));

    if (x == NULL)
        throw "Memory allocation failure";

    /*
     * Store the data
     */
    x->name = new std::string(name);
    x->type = CONFIG_INTEGER;
    x->value.value = new int(val);

    /*
     * Store the entry.
     */
    m_entries[name] = x;

    /*
     * Notify our global state of the variable change.
     */
    if (notify)
        notify_watchers(name);
}


/*
 * Set the given key to the array of strings.
 *
 * This replaces any prior value which might have been stored under that key.
 */
void CConfig::set(std::string name, std::vector < std::string > entries, bool notify)
{

    /*
     * Delete any existing value stored under this key.
     */
    delete_key(name);

    /*
     * Create the new entry.
     */
    CConfigEntry *x = (CConfigEntry *) malloc(sizeof(CConfigEntry));

    if (x == NULL)
        throw "Memory allocation failure";

    /*
     * Store the data.
     */
    x->name = new std::string(name);
    x->type = CONFIG_ARRAY;
    x->value.array = new std::vector < std::string >;

    /*
     * Copy the string-values.
     */
    for (auto it = entries.begin(); it != entries.end(); ++it)
    {
        x->value.array->push_back((*it));
    }

    /*
     * Add the entry.
     */
    m_entries[name] = x;

    /*
     * Notify our global state of the variable change.
     */
    if (notify)
        notify_watchers(name);
}

/*
 * Helper to get the integer-value of a named key.
 */
int CConfig::get_integer(std::string name, int default_value)
{
    int result = 0;

    CConfigEntry *tmp = m_entries[name];

    if (tmp && (tmp->type == CONFIG_INTEGER))
        result = *tmp->value.value;
    else
        result = default_value;

    return (result);
}


/*
 * Helper to get the string-value of a named key.
 */
std::string CConfig::get_string(std::string name, std::string default_value)
{
    std::string result;

    CConfigEntry *tmp = m_entries[name];

    if (tmp && (tmp->type == CONFIG_STRING))
        result = *tmp->value.str;
    else
        result = default_value;

    return (result);
}


/*
 * Helper to get the array-value of a named key.
 */
std::vector<std::string> CConfig::get_array(std::string name)
{
    std::vector<std::string> result;

    CConfigEntry *tmp = m_entries[name];

    if (tmp && (tmp->type == CONFIG_ARRAY))
        result = *tmp->value.array;

    return result;
}


void CConfig::notify_watchers(std::string key_name)
{
    int max = views.size();

    for (int i = 0; i < max; i++)
        views[i]->update(key_name);
}
