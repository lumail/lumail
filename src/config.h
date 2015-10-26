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

#include <vector>
#include <string>


/**
 * Our config-type can contain "string" or "array" values.
 *
 * Setup a type for that.
 */
typedef enum
{ CONFIG_UNKNOWN, CONFIG_STRING, CONFIG_ARRAY } configType;


/**
 * This is the struct which holds a single configuration value.
 *
 * The value might be a string, or an array of strings.
 */
struct CConfigEntry
{
    /**
     * The name of the configuration-option.
     */
    std::string * name;

    /**
     * The type of the configuration-option: STRING vs ARRAY
     */
    configType type;

    /**
     * The actual value of this entry, stored as a union.
     */
    union
    {
        std::string * str;
        std::vector < std::string > *array;
    } value;

};



/**
 * Singleton to hold configuration variables.
 */
class CConfig
{
private:
    CConfig();
    ~CConfig();

public:

    /**
     * Instance accessor - this is a singleton.
     */
    static CConfig *instance();

    /**
     * Get the value associated with a name.
     */
    CConfigEntry *get(std::string name);

    /**
     * Get all the keys we know about.
     */
    std::vector < std::string > keys();

    /**
     * Set a configuration key to contain the specified value.
     */
    void set(std::string name, std::string value, bool notify = true);

    /**
     * Set a configuration key to contain the specified array-value.
     */
    void set(std::string name, std::vector < std::string > entries, bool notify  = true);


    /**
     * Helper to get the string-value of a named key.
     */
    std::string get_string(std::string name);

    /**
     * Helper to get the array-value of a named key.
     */
    std::vector<std::string> get_array(std::string name);

private:

    /**
     * Remove the value of the given key.
     */
    void delete_key(std::string key);

private:

    /**
     * The actual storage.
     */
    std::vector < CConfigEntry * >m_entries;
};
