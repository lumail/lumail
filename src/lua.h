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


extern "C"
{
# include <lua.h>
# include <lauxlib.h>
# include <lualib.h>
}

#include <vector>




/**
 * A structure for storing the mapping between Lua-function names,
 * and their C-implementations.
 */
struct CLuaMapping
{
    /**
     * The name of the primitive, as used by Lua.
     */
    const char *name;

    /**
     * A one-line description of the function.
     */
    const char *help;

    /**
     * The implementation of the primitive, in C.
     */
    lua_CFunction func;
};




/**
 * A list of all the primitives we export to Lua.
 */
extern struct CLuaMapping primitive_list[];




/**
 * The number of primitives we've exported.
 */
extern int primitive_count;




/**
 * A singleton class holding a Lua interpreter.
 *
 * This is used to invoke Lua functions/methods from the rest of the code,
 * and to lookup keybindings, etc.
 *
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
     * Lookup a value in a nested table.
     *
     * (Used for keybinding lookups.)
     */
    char *get_nested_table( std::string table, std::string key, std::string subkey );

    /**
     * Execute a function from the global keymap.
     */
    bool on_keypress(const char *keypress );

    /**
     * Invoke the on_key() lua-defined callback.
     */
    bool on_key(const char *key );

    /**
     * Call the completion function from Lua.
     */
    std::vector<std::string> on_complete();

    /**
     * convert a table to an array of strings.
     */
    std::vector<std::string> table_to_array( std::string name );

    /**
     * Retrieve the value of a global boolean variable.
     */
    bool get_bool( std::string name, bool default_value = false );

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
     * The handle to the Lua interpreter.
     */
    lua_State *m_lua;

};

#endif /* _clua_h_ */
