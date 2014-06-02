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

#pragma once

extern "C"
{
# include <lua.h>
# include <lauxlib.h>
# include <lualib.h>
}

#include <vector>
#include <memory>
#include "utfstring.h"


class CMaildir;



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
 * We've defined functions here which are C++ equivilents to the Lua primitives
 * this is helpful to consolidate code, even though the C++ versions aren't
 * really necessary.
 *
 * (For example we could invoke the "get_signature" method by messing with the
 * Lua_State object and pushing/popping the stack directly but having a C++-visible
 * function avoids that, and is neater.)
 *
 */
class CLua
{

public:

/**
 ** Object methods.
 **/

    /**
     * Get access to this singleton instance.
     */
    static CLua *Instance();

    /**
     * Load the specified Lua file, and evaluate it.
     */
    bool load_file(std::string filename);

    /**
     * Evaluate the given string.
     */
    void execute(std::string lua, bool show_error = true);

    /**
     * Lookup a value in a nested-table.
     *
     * (Used for keybinding lookups.)
     */
    char *get_nested_table( std::string table, std::string key, std::string subkey );


    /**
     * Convert a Lua table to an array of strings.
     */
    std::vector<std::string> table_to_array( std::string name );

    /**
     * Return the value of the Lua-defined boolean variable.
     */
    bool get_bool( std::string name, bool default_value = false );


/**
 ** Helper methods.
 **/

    /**
     * Dump the Lua stack contents, in a debug-build.
     */
    void dump_stack();

    /**
     * Read a single line of text, via the Lua prompt function.
     */
    UTFString get_input( UTFString prompt_txt, UTFString default_answer = "" );

    /**
     * Get the MIME-type of a given file.  Using the suffix-only.
     */
    std::string get_mime_type( std::string filename );

    /**
     * Call the "get_signature" hook.
     */
    UTFString get_signature( UTFString from, UTFString to, UTFString subject );

    /**
     * Invoke the "on_complete" callback, which might be defined by the user.
     */
    std::vector<std::string> on_complete();

    /**
     * Invoke the on_get_body() callback.
     */
    std::vector<UTFString> on_get_body();

    /**
     * Invoke the Lua-defined on_key() callback.
     */
    bool on_key(const char *key );

    /**
     * Execute a function from the global keymap.
     */
    bool on_keypress(const char *keypress );

    /**
     * Does the named function exist?
     */
    bool is_function( const char *name );

    /**
     * Call a global function, passing a CMaildir, and return the
     * result as converted to boolean using Lua semantics, ie only
     * false and nil are not true.
     *
     * The named Lua function should return a true value to include the
     * CMaildir.
     *
     * On an error, returns the onerror value, which defaults to true
     * so that folders aren't accidentally hidden by a Lua error.
     */
    bool filter(const char *name, std::shared_ptr<CMaildir> maildir,
                bool onerror=true);

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
