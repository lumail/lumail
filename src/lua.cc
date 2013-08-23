/**
 * lua.cc - Singleton interface to an embedded Lua interpreter.
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


#include <cstdlib>
#include <fstream>
#include <string.h>
#include <stdlib.h>


#include "bindings.h"
#include "variables.h"
#include "debug.h"
#include "file.h"
#include "global.h"
#include "lua.h"
#include "version.h"


/**
 * Instance-handle.
 */
CLua *CLua::pinstance = NULL;



/**
 * This is a list of all the functions that we export to our
 * embedded Lua-interpreter.
 *
 * The struct is externally declared in our header so that we
 * can access it from elsewhere.
 *
 * (Specifically we want to make the function-names available to
 * our TAB-completion code.
 *
 */
struct CLuaMapping primitive_list[] =
{

/**
 * Basic primitives: defined in src/primitives.cc
 */
    {"abort", "Exit lumail with an error message.", (lua_CFunction) abort },
    {"alert", "Show an alert which requires confirmation.", (lua_CFunction) alert },
    {"clear", "Clear the screen.", (lua_CFunction) clear },
    {"dump_stack", "Dump the Lua-stack for debugging purposes", (lua_CFunction) lua_dump_stack },
    {"exec", "Execute an external command.", (lua_CFunction) exec },
    {"exit", "Exit lumail.", (lua_CFunction) exit },
    {"help", "Show brief help for primitives.", (lua_CFunction) show_help },
    {"history_file", "The path to log history to.", (lua_CFunction) history_file },
    {"log_message", "Add a message to the debug-log.", (lua_CFunction) log_message },
    {"mime_type", "Get the MIME-type for a file.", (lua_CFunction) mime_type },
    {"msg", "Write a message to the status-area.", (lua_CFunction) msg },
    {"refresh_display", "Refresh the display.", (lua_CFunction) refresh_display },
    {"screen_height", "Return the height of the screen in rows.", (lua_CFunction) screen_height },
    {"screen_width", "Return the width of the screen in columns.", (lua_CFunction) screen_width },
    {"sleep", "Pause execution for the given number of seconds.", (lua_CFunction) sleep },
    {"stuff", "Stuff keys into the input-buffer", (lua_CFunction) stuff },

/**
 * File/Path utilities.  Defined in src/bindings_file.cc
 */
    {"create_maildir", "Create a new maildir.", (lua_CFunction) create_maildir },
    {"executable", "Is the given file executable?", (lua_CFunction) executable },
    {"file_exists", "Does the given file exist?", (lua_CFunction) file_exists },
    {"is_directory", "Is the given path a directory?", (lua_CFunction) is_directory },
    {"is_maildir", "Is the given path a maildir?", (lua_CFunction) is_maildir },
    {"load_directory", "Load *.lua from beneath the given directory.  (Not recursively.)", (lua_CFunction) load_directory },

/**
 * Folder Selection: defined in src/bindings_folders.cc
 */
    {"add_selected_folder", "Add the current folder to the list of selected folders", (lua_CFunction) add_selected_folder },
    {"clear_selected_folders", "Clear the list of selected-folders.", (lua_CFunction) clear_selected_folders },
    {"selected_folders", "Return the currently selected folders.", (lua_CFunction) selected_folders },
    {"set_selected_folder", "Remove all currently selected folders and add the single named one to the set.", (lua_CFunction) set_selected_folder },
    {"toggle_selected_folder", "Toggle the folder into/out-of the selected set.", (lua_CFunction) toggle_selected_folder },

/**
 * Get/Set variables: defined in src/variables.cc
 */
    {"completion_chars", "Get/set the characters to tokenize on for completion.", (lua_CFunction) completion_chars },
    {"editor", "Query or update the editor to use.", (lua_CFunction) editor },
    {"from", "Query or update the from-address for outgoing mails.", (lua_CFunction) from },
    {"global_mode", "Query or update the global-mode.", (lua_CFunction) global_mode },
    {"index_format", "Query or update the index-format string.", (lua_CFunction) index_format },
    {"index_limit", "Query or update the index-limit string.", (lua_CFunction) index_limit },
    {"maildir_format", "Query or update the maildir-format string.", (lua_CFunction) maildir_format },
    {"maildir_limit", "Query or update the maildir-limit string.", (lua_CFunction) maildir_limit },
    {"maildir_prefix", "Query or update the root of the Maildir hierarchy.", (lua_CFunction) maildir_prefix },
    {"display_filter", "Query or update the filter to apply to messages being viewed.", (lua_CFunction) display_filter },
    {"mail_filter", "Query or update the filter to apply to messages being processed.", (lua_CFunction) mail_filter },
    {"sendmail_path", "Query or update the sendmail-path, used for sending mails.", (lua_CFunction) sendmail_path },
    {"sent_mail", "Query or update the Maildir location to send outgoing mails to.", (lua_CFunction) sent_mail },
    {"sort", "Query or update the sorting string for index-mode.", (lua_CFunction) sort },
    {"get_variables", "Retrieve all known-variables and their values.", (lua_CFunction) get_variables },

/**
 * Colour & highlight getters/setters.  Defined in src/variables.cc
 */
    {"attachment_colour", "Get/Set the colour to use for drawing attachments.", (lua_CFunction) attachment_colour },
    {"body_colour", "Get/Set the colour to use for drawing message-bodies.", (lua_CFunction) body_colour },
    {"header_colour", "Get/Set the colour to use for drawing message-headers.", (lua_CFunction) header_colour },
    {"unread_maildir_colour", "Get/Set the colour to use for drawing Maildirs which contain unread messages.", (lua_CFunction) unread_maildir_colour },
    {"unread_message_colour", "Get/Set the colour to use for drawing unread messages.", (lua_CFunction) unread_message_colour },
    {"maildir_highlight_mode", "Get/Set maildir highlight mode.", (lua_CFunction) maildir_highlight_mode },
    {"index_highlight_mode", "Get/Set index highlight mode.", (lua_CFunction) index_highlight_mode },

/**
 * Index functions: defined in src/bindings_index.cc
 */
    {"index_offset", "Get the current offset into the index.", (lua_CFunction) index_offset},
    {"jump_index_to", "Jump to the named offset in the message list.", (lua_CFunction) jump_index_to },
    {"scroll_index_down", "Scroll the message list down.", (lua_CFunction) scroll_index_down },
    {"scroll_index_to", "Scroll the message list to the given offset.", (lua_CFunction) scroll_index_to },
    {"scroll_index_up", "Scroll the message list up.", (lua_CFunction) scroll_index_up },

/**
 * Message-Related functions: defined in src/bindings_message.cc
 */
    {"body", "Retrieve the body of the current message.", (lua_CFunction) body },
    {"compose", "Compose a new outgoing email.", (lua_CFunction) compose },
    {"count_lines", "Count the number of lines in the message body", (lua_CFunction) count_lines},
    {"count_messages", "Count the messages in the currently selected Maildir(s).", (lua_CFunction) count_messages },
    {"current_message", "Retrieve the path to the current message.", (lua_CFunction) current_message },
    {"delete", "Delete the current message.", (lua_CFunction) delete_message },
    {"header", "Retrieve the value of the given header from the current message.", (lua_CFunction) header },
    {"is_new", "Is the current message new/unread?", (lua_CFunction) is_new },
    {"mark_read", "Mark the current message as old/read.", (lua_CFunction) mark_read },
    {"mark_unread", "Mark the current message as unread.", (lua_CFunction) mark_unread },
    {"message_offset", "The offset within the message we've scrolled to", (lua_CFunction) message_offset },
    {"reply", "Reply to the current message.", (lua_CFunction) reply },
    {"save", "Save the current message in a new location, and delete it.", (lua_CFunction) save_message },
    {"save_message", "Save the current message in a new location, and delete it.", (lua_CFunction) save_message },
    {"scroll_message_down", "Scroll the current message down.", (lua_CFunction) scroll_message_down },
    {"scroll_message_to", "Scroll the current message to the given offset.", (lua_CFunction) scroll_message_to },
    {"scroll_message_up", "Scroll the current message up.", (lua_CFunction) scroll_message_up },
    {"send_email", "Send an email, via Lua.", (lua_CFunction) send_email },

/**
 * Maildir-related functions, defined in bindings_maildirs.cc
 */
    {"count_maildirs", "Count the visible Maildirs.", (lua_CFunction) count_maildirs },
    {"current_maildir", "Return the path to the current Maildir.", (lua_CFunction) current_maildir },
    {"current_maildirs", "Get the names of all currently visible Maildirs.", (lua_CFunction) current_maildirs },
    {"jump_maildir_to", "Jump the Maildir list to the given offset.", (lua_CFunction) jump_maildir_to },
    {"maildir_offset", "Get the current offset into the maildir list.", (lua_CFunction) maildir_offset},
    {"maildirs_matching", "Return the maildirs matching the given regexp.", (lua_CFunction) maildirs_matching },
    {"scroll_maildir_down", "Scroll the maildir list down.", (lua_CFunction) scroll_maildir_down },
    {"scroll_maildir_to", "Scroll the maildir to the next entry matching the given regexp.", (lua_CFunction) scroll_maildir_to },
    {"scroll_maildir_up", "Scroll the maildir list up.", (lua_CFunction) scroll_maildir_up },
    {"select_maildir", "Select a Maildir by path.", (lua_CFunction) select_maildir },


/**
 * Prompts.  Defined in src/bindings_prompts.cc
 */
    {"choose_string", "Prompt for one of a small set of strings", (lua_CFunction) choose_string},
    {"prompt", "Prompt for input.", (lua_CFunction) prompt },
    {"prompt_chars", "Prompt for input, until one of a given number of characters is entered.", (lua_CFunction) prompt_chars },
    {"prompt_maildir", "Prompt for an (existing) Maildir.", (lua_CFunction) prompt_maildir },
    {"prompt_yn", "Prompt for a yes/no answer.", (lua_CFunction) prompt_yn },

/**
 * Attachments & body-parts Defined in src/bindings_mime.cc
 */
    {"attachments", "Retrieve the list of attachments in the current message.", (lua_CFunction) attachments },
    {"count_attachments", "Count the number of attachments the current message contains.", (lua_CFunction) count_attachments },
    {"count_body_parts", "Return the count of body-parts", (lua_CFunction) count_body_parts },
    {"get_body_parts", "Get the body-parts of the message", (lua_CFunction) get_body_parts },
    {"get_body_part", "Retrieve the given body-part from the message.", (lua_CFunction) get_body_part },
    {"has_body_part", "Does the message have a part of the given MIME-type?", (lua_CFunction) has_body_part },
    {"save_attachment", "Save the given attachment to disk, from the current message.", (lua_CFunction) save_attachment },

};


/**
 * The size of our primitives table
 */
int primitive_count = (sizeof(primitive_list)/sizeof(struct CLuaMapping));



/**
 * Get access to this singleton object.
 */
CLua *CLua::Instance()
{
    if (!pinstance)
        pinstance = new CLua;

    return pinstance;
}



/**
 * Constructor - This is private as this class is a singleton.
 */
CLua::CLua()
{
    /**
     * Create a new Lua object.
     */
    m_lua = luaL_newstate();


    /**
     * Register the libraries.
     */
    luaopen_base(m_lua);
    luaL_openlibs(m_lua);


    /**
     * Now register our primitives.
     */
    for(int i = 0; i < primitive_count; i++ )
    {
        lua_pushcfunction(m_lua, primitive_list[i].func);
        lua_setglobal(m_lua, primitive_list[i].name);
    }


    /**
     * Set a global variable into the Lua environment.
     */
    lua_pushstring(m_lua, LUMAIL_VERSION );
    lua_setglobal(m_lua, "VERSION" );

    /**
     * Set a global variable into the Lua environment.
     */
#ifdef LUMAIL_DEBUG
    lua_pushboolean(m_lua, 1 );
#else
    lua_pushboolean(m_lua, 0 );
#endif
    lua_setglobal(m_lua, "DEBUG" );


    /**
     * Load a panic-handler - which will call abort()
     */
    lua_atpanic(m_lua, abort);
}


/**
 * Load the specified Lua file, and evaluate it.
 */
bool CLua::load_file(std::string filename)
{
#ifdef LUMAIL_DEBUG
        std::string dm = "CLua::load_file(\"";
        dm += filename;
        dm += "\");";
        DEBUG_LOG( dm );
#endif

    if (CFile::exists( filename ) )
    {
        if (luaL_loadfile(m_lua, filename.c_str())
            || lua_pcall(m_lua, 0, 0, 0))
        {
            fprintf(stderr, "Failed to load/parse/execute %s: %s",
                    filename.c_str(),
                    lua_tostring(m_lua, -1));
            exit(1);
        }
        return true;
    }
    return false;
}


/**
 * Evaluate the given string.
 */
void CLua::execute(std::string lua)
{
    if ( luaL_dostring(m_lua, lua.c_str()))
    {

#ifdef LUMAIL_DEBUG
        std::string dm = "CLua::execute(\"";
        dm += lua;
        dm += "\"); -> ";

        const char *err = lua_tostring(m_lua, -1);
        dm += err;
        DEBUG_LOG( dm );
#endif
    }
}



/**
 * Lookup a value in a nested-table.
 *
 * (Used for keybinding lookups.)
 */
char *CLua::get_nested_table( std::string table, std::string key, std::string subkey )
{
    char *result = NULL;
    static char quit_string[] =  {"exit();"};

    /**
     * Ensure the table exists.
     */
    lua_getglobal(m_lua, table.c_str() );
    if (lua_isnil (m_lua, -1 ) )
    {
        /**
         * If the table doesn't exist we'll return "exit();",
         * if the keypress was one of: q/Q/x/X
         *
         * We can do this here because we don't call the function
         * from elsewhere.
         */
        if ( ( subkey.size() > 0 ) &&
             ( ( subkey.at(0) == 'q' ) ||
               ( subkey.at(0) == 'Q' ) ||
               ( subkey.at(0) == 'x' ) ||
               ( subkey.at(0) == 'X' ) ) )
            result = quit_string;

        return result;
    }

    /**
     * Get the sub-table.
     */
    lua_pushstring(m_lua, key.c_str() );
    lua_gettable(m_lua, -2);
    if (! lua_istable(m_lua, -1 ) )
        return result;

    /**
     * Get the value.
     */
    lua_pushstring(m_lua, subkey.c_str());
    lua_gettable(m_lua, -2);
    if (lua_isnil (m_lua, -1 ) )
        return result;

    /**
     * If it worked, and is a string .. return it.
     */
    if (lua_isstring(m_lua, -1))
        result = (char *)lua_tostring(m_lua, -1);
    return result;
}


/**
 * Look up the binding for the named keystroke in our keymap(s).
 *
 * If the result is a string then execute it as a function.
 */
bool CLua::on_keypress(const char *keypress)
{
    /**
     * The result of the lookup.
     */
    char *result = NULL;

    /**
     * Get the current global-mode.
     */
    CGlobal *global   = CGlobal::Instance();
    std::string *mode = global->get_variable("global_mode");

    /**
     * Lookup the keypress in the current-mode-keymap.
     */
    result = get_nested_table( "keymap", mode->c_str(), keypress );


    /**
     * If that failed then lookup the global keymap.
     *
     * This order ensures you can have a "global" keymap, overridden in just one mode.
     */
    if ( result == NULL )
        result = get_nested_table( "keymap", "global", keypress );

    /**
     * If one/other of these lookups resulted in success then we're golden.
     */
    if ( result != NULL )
        execute(result);

    /**
     * We succeeded if the result wasn't NULL.
     */
    return( result != NULL );
}


/**
 * Invoke the Lua-defined on_key() callback.
 */
bool CLua::on_key(const char *key )
{
    /**
     * Get the "on_key()" function, and see if it exists.
     */
    lua_getglobal( m_lua, "on_key" );
    if(!lua_isfunction(m_lua,-1))
        return false;


    /**
     * Push the key and call it.
     */
    lua_pushstring( m_lua, key );
    lua_pcall(m_lua, 1, 1 , 0 ) ;

    /**
     * Get the return value.
     */
    int ret = lua_tointeger(m_lua,-1);
    lua_pop(m_lua,1);
    if ( ret )
        return true;

    /**
     * Fail
     */
    return false;
}


/**
 * Invoke the on_get_body() callback.
 */
std::vector<UTFString> CLua::on_get_body()
{
    std::vector<UTFString> result;

    /**
     * Get the "on_get_body()" function, and see if it exists.
     */
    lua_getglobal( m_lua, "on_get_body" );
    if(!lua_isfunction(m_lua,-1))
        return( result );


    /**
     * Call the function
     */
    lua_pcall(m_lua, 0, 1, 0 ) ;

    /**
     * Get the return value.
     */
    const char *str = lua_tostring(m_lua,-1);
    lua_pop(m_lua,1);

    /**
     * If the string is non-empty then parse.
     */
    if ( str != NULL && strlen( str ) )
    {
        /**
         * Split the body into an array, by newlines.
         */
        std::stringstream stream(str);
        std::string line;
        while (std::getline(stream, line))
        {
            result.push_back( line );
        }
    }


    return( result );
}


/**
 * Invoke the "on_complete" callback, which might be defined by the user.
 */
std::vector<std::string> CLua::on_complete()
{
    std::vector<std::string> results;

    /**
     * If the global isn't defined then we return an empty set.
     */
    lua_getglobal( m_lua, "on_complete" );
    if(!lua_isfunction(m_lua,-1))
        return results;

    /**
     * Call it.
     */
    lua_pcall(m_lua, 0, 1 , 0 ) ;


    if (lua_type(m_lua, -1)!=LUA_TTABLE)
    {
        lua_pop(m_lua, 1);
        return results;
    }

    lua_pushnil(m_lua);

    while (lua_next(m_lua, -2))
    {
        const char *d  = lua_tostring(m_lua, -1);
        results.push_back( d );
        lua_pop( m_lua , 1);
    }

    /**
     * Cleanup the table.
     */
    lua_pop(m_lua,1);

    return( results );
}


/**
 * Convert a Lua table to an array of strings.
 */
std::vector<std::string> CLua::table_to_array( std::string name )
{
    std::vector<std::string> results;

#ifdef LUMAIL_DEBUG
    std::string ds = "table_to_array(";
    ds += name;
    ds += ") - start";
    DEBUG_LOG( ds );
#endif

    /**
     * Ensure we have a table.
     */
    lua_getglobal(m_lua, name.c_str() );
    if (lua_type(m_lua, -1)!=LUA_TTABLE)
    {
        lua_pop(m_lua, 1);

#ifdef LUMAIL_DEBUG
    std::string ds = "table_to_array() -> Not a table";
    DEBUG_LOG( ds );
#endif

        return results;
    }

    lua_pushnil(m_lua);


    while (lua_next(m_lua, -2))
    {
        const char *d  = lua_tostring(m_lua, -1);

#ifdef LUMAIL_DEBUG
        std::string dm = "\tFound:";
        dm += d;
        DEBUG_LOG( dm );
#endif

        results.push_back( d );
        lua_pop( m_lua , 1);
    }


    /**
     * Cleanup the table.
     */
    lua_pop(m_lua,1);

#ifdef LUMAIL_DEBUG
    std::string de = "table_to_array(";
    de += name;
    de += ") - end";
    DEBUG_LOG( de );
#endif


    return( results );
}


/**
 * Return the value of the Lua-defined boolean variable.
 */
bool CLua::get_bool( std::string name, bool default_value )
{
    bool ret = default_value;
    lua_getglobal(m_lua, name.c_str() );
    if (lua_type(m_lua, -1) != LUA_TBOOLEAN )
    {
        lua_pop(m_lua, 1);
        return ret;
    }
    ret = lua_toboolean(m_lua,-1);
    lua_pop(m_lua,1);
    return ret;

}

/**
 * Get the MIME-type of a given file.  Using the suffix-only.
 */
std::string CLua::get_mime_type( std::string filename )
{
#ifdef LUMAIL_DEBUG
    DEBUG_LOG( "CLua::get_mime_type(" + filename + ")" );
#endif

    lua_pushstring(m_lua, filename.c_str() );
    if ( mime_type( m_lua ) != 1 )
        return "application/octet-stream";
    const char *type = lua_tostring(m_lua,-1);

#ifdef LUMAIL_DEBUG
    DEBUG_LOG( "CLua::get_mime_type(" + filename + ") -> " + std::string( type ) );
#endif
    return( type );
}

/**
 * Dump the Lua stack contents - only in debug-builds.
 */
void CLua::dump_stack()
{
#ifdef LUMAIL_DEBUG

    DEBUG_LOG( "dump_stack() - start" );

    /**
     * Top of the stack.
     */
    int top = lua_gettop(m_lua);

    /* Repeat for each level */
    for (int i = 1; i <= top; i++)
    {
        char buf[1024]={'\0'};

        int t = lua_type(m_lua, i);
        switch (t)
        {
        case LUA_TSTRING:
            snprintf(buf, sizeof(buf)-1,"`%s'", lua_tostring(m_lua, i));
            break;

        case LUA_TBOOLEAN:
            snprintf(buf, sizeof(buf)-1,lua_toboolean(m_lua, i) ? "true" : "false");
            break;

        case LUA_TNIL:
            snprintf(buf, sizeof(buf)-1,"nil");
            break;

        case LUA_TNUMBER:
            snprintf(buf, sizeof(buf)-1,"%g", lua_tonumber(m_lua, i));
            break;

        default:
            snprintf(buf, sizeof(buf)-1,"%s", lua_typename(m_lua, t));
            break;

        }

        DEBUG_LOG( buf );
    }

    DEBUG_LOG( "dump_stack() - end" );
#endif

}

