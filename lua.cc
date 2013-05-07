/**
 * lua.cc - Singleton interface to an embedded Lua interpreter.
 */


#include <iostream>
#include <cstdlib>
#include <fstream>
#include <sys/stat.h>
#include <sys/types.h>
#include <string.h>
#include <malloc.h>

#include "lua.h"
#include "bindings.h"
#include "global.h"
#include "version.h"



/**
 * Instance-handle.
 */
CLua *CLua::pinstance = NULL;



/**
 * Get access to our LUA intepreter.
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
   * Create LUA object.
   */
    m_lua = lua_open();
    luaopen_base(m_lua);
    luaL_openlibs(m_lua);

    /**
     * Version number
     */
    setGlobal("VERSION", LUMAIL_VERSION);

    /**
     * Register our primitives.
     */
    lua_register(m_lua, "clear", clear);
    lua_register(m_lua, "msg", msg);
    lua_register(m_lua, "exec", exec);
    lua_register(m_lua, "prompt", prompt);

    /**
     * Get/Set the global mode.
     */
    lua_register(m_lua, "global_mode", global_mode);

    /**
     * Get/Set the maildir prefix.
     */
    lua_register(m_lua, "maildir_prefix", maildir_prefix);

    /**
     * Scroll mailboxes up/down.
     */
    lua_register(m_lua, "scroll_maildir_down", scroll_maildir_down );
    lua_register(m_lua, "scroll_maildir_up", scroll_maildir_up );

    /**
     * Find the next folder matching the pattern.
     */
    lua_register(m_lua, "scroll_maildir_to", scroll_maildir_to );
    lua_register(m_lua, "current_maildir", current_maildir );

    /**
     * Folder selections.
     */
    lua_register(m_lua, "clear_selected_folders", clear_selected_folders );
    lua_register(m_lua, "add_selected_folder", add_selected_folder );

    /**
     * Get/Set the maildir-limit.
     */
    lua_register( m_lua, "maildir_limit", maildir_limit );

    lua_register(m_lua, "exit", exit);
}



/**
 * Load the specified lua file, and evaluate it.
 */
void CLua::loadFile(std::string filename)
{
    struct stat sb;

    std::cout << "Loading file " << filename << std::endl;
    if ((stat(filename.c_str(), &sb) == 0)) {
	if (luaL_loadfile(m_lua, filename.c_str())
	    || lua_pcall(m_lua, 0, 0, 0)) {
	    fprintf(stderr, "cannot run configuration file: %s",
		    lua_tostring(m_lua, -1));
	    exit(1);
	}
    }
}


/**
 * Evaluate the given string.
 */
void CLua::execute(std::string lua)
{
    luaL_dostring(m_lua, lua.c_str());
}


/**
 * Call a single Lua function, passing no arguments and ignoring the return code.
 */
bool CLua::callFunction(std::string name)
{
    lua_getglobal(m_lua, name.c_str());
    if (lua_isfunction(m_lua, -1)) {
	lua_pcall(m_lua, 0, 0, 0);
	return true;
    } else {
	return false;
    }
}

/**
 * Set a global variable into the Lua environment.
 */
void CLua::setGlobal(std::string name, std::string value)
{
    lua_pushstring(m_lua, value.c_str());
    lua_setglobal(m_lua, name.c_str());
}


/**
 * Get a global variable value from the Lua environment.
 */
std::string * CLua::getGlobal(std::string name)
{
    std::string * result = NULL;

    lua_getglobal(m_lua, name.c_str());
    if (!lua_isnil(m_lua, -1))
	result = new std::string(lua_tostring(m_lua, -1));

    lua_pop(m_lua, 1);

    return result;
}

/**
 * Execute a function from the global keymap.
 */
bool CLua::onKey( char key )
{
  char keypress[2] ;
  keypress[0] = key;
  keypress[1] = '\0';

  /**
   * Get the mode.
   */
  CGlobal     *g = CGlobal::Instance();
  std::string *s = g->get_mode();

  lua_getglobal (m_lua, "keymap");
  lua_pushstring (m_lua, s->c_str());
  lua_gettable (m_lua, -2);
  lua_pushstring (m_lua, keypress);
  lua_gettable (m_lua, -2);
  if (lua_isstring (m_lua, -1))
    {
      char *str = (char *) lua_tostring (m_lua, -1);
      execute( str );
      return true;
    }

  return false;
}
