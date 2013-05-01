/**
 * lua.cc - Singleton interface to an embedded Lua interpreter.
 */


#include <iostream>
#include <fstream>
#include <sys/stat.h>
#include <sys/types.h>
#include <string.h>
#include <malloc.h>

#include "lua.h"
#include "version.h"


extern int my_function(lua_State * L);



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
    setGlobal("VERSION", LUMAIL_VERSION );

    lua_register(m_lua, "my_function", my_function);
}



/**
 * Load the specified lua file, and evaluate it.
 */
void CLua::loadFile(std::string filename)
{
    struct stat sb;

    if ((stat(filename.c_str(), &sb) == 0))
	if (0 == luaL_loadfile(m_lua, (char *) (filename.c_str())))
	    lua_pcall(m_lua, 0, 0, 0);
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
