/**
 * lua.h - Singleton interface to an embedded Lua interpreter.
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


#ifndef _clua_h_
#define _clua_h_ 1


extern "C" {
# include <lua.h>
# include <lauxlib.h>
# include <lualib.h>
}

#include <vector>


/**
 * A singleton class holding a Lua intepreter.
 */
class CLua
{
public:

    /**
     * Get access to the singleton instance.
     */
    static CLua *Instance();

    /**
     * Load and execute a single file of Lua code.
     */
    bool load_file(std::string filename);

    /**
     * Evaluate the given string.
     */
    void execute(std::string lua);

    /**
     * Call a single Lua function, passing no arguments and ignoring the return code.
     */
    bool call_function(std::string name);

    /**
     * Lookup a value in a nested table.
     */
    char *get_nested_table( std::string table, std::string key, std::string subkey );

    /**
     * Execute a function from the global keymap.
     */
    bool on_keypress(const char *keypress );

    /**
     * Call the lua-function on_key()
     */
    bool on_key(const char *key );

    /**
     * convert a table to an array of strings.
     */
    std::vector<std::string> table_to_array( std::string name );

    /**
     * Dump the stack contents - only in debug-builds.
     */
    void dump_stack();

 protected:

    /**
     * Protected functions to allow our singleton implementation.
     */
    CLua();
    CLua(const CLua &);
    CLua & operator=(const CLua &);

 private:

    /**
     * The single instance of this class.
     */
    static CLua *pinstance;

    /**
     * The handle to the lua intepreter.
     */
    lua_State *m_lua;

};

#endif /* _clua_h_ */
