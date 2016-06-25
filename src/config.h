/*
 * config.h - A configuration-value holding class
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
#include <unordered_map>

#include "observer.h"
#include "singleton.h"

/**
 * The CConfig class holds configuration values, these values
 * might be arrays, integers, or strings.
 *
 * We use this enum to identify which type a particular entry has.
 */
typedef enum
{ CONFIG_UNKNOWN, CONFIG_STRING, CONFIG_INTEGER, CONFIG_ARRAY } configType;


/**
 * This is the struct which holds a single configuration value.
 *
 * The value held might be an array of strings, an integer, or string.
 */
struct CConfigEntry
{
    /**
     * The name of this configuration-option.
     */
    std::string * name;

    /**
      * The type of the configuration-option.
      */
    configType type;

    /**
      * The actual value of this entry, stored as a union.
      */
    union
    {
        std::string * str;
        std::vector < std::string > *array;
        int *value;
    } value;

};



/**
 * This is a singleton class which is used to get/set configuration
 * values.
 *
 * It also implements the Subject interface of the Observer design-pattern,
 * allowing other objects to listen to changes.  We broadcast change events
 * when the value of a given key has changed.  Each broadcast contains only
 * the name of the key which has been set/updated.
 */
class CConfig : public Singleton<CConfig>, public Subject
{
public:
    /**
     * Constructor.
     */
    CConfig();

    /**
     * Destructor - Free the memory associated with each configuration value.
     */
    ~CConfig();

public:

    /**
     * Get the value associated with a name.
     */
    CConfigEntry *get(std::string name);

    /**
     * Get all the keys we know about.
     */
    std::vector < std::string > keys();

    /**
     * Set a configuration key to contain the specified string value.
     */
    void set(std::string name, std::string value, bool notify = true);

    /**
     * Set a configuration key to contain the specified integer value.
     */
    void set(std::string name, int value, bool notify = true);

    /**
     * Set a configuration key to contain the specified array-value.
     */
    void set(std::string name, std::vector < std::string > entries, bool notify  = true);

    /**
     * Helper to get the array-value of a named key.
     */
    std::vector<std::string> get_array(std::string name);

    /**
     * Helper to get the integer-value of a named key.
     *
     * If the value is not found the supplied default will be used instead.
     */
    int get_integer(std::string name, int default_value = 0);

    /**
     * Helper to get the string-value of a named key.
     *
     * If the value is not found the supplied default will be used instead.
     */
    std::string get_string(std::string name, std::string default_value = "");

    /**
     * Delete all keys and their associated values.
     */
    void remove_all();


    /**
     * Remove the value of the given key, freeing the associated
     * CConfigEntry structure.
     */
    void delete_key(std::string key);

private:

    /**
     * Notify any watchers that the value of a configuration-key
     * has changed.  This is implemented via the Observer pattern.
     */
    void notify_watchers(std::string key_name);

    /**
     * The actual map which stores our configured names & value pairs.
     */
    std::unordered_map <std::string, CConfigEntry * >m_entries;
};
