
#include <cstdlib>

#include "lua.h"


/**
 * External functions implemented in *_lua.cc
 */
extern void InitConfig (lua_State * l);
extern void InitMaildir (lua_State * l);
extern void InitMessage (lua_State * l);
extern void InitMessagePart (lua_State * l);
extern void InitScreen (lua_State * l);




/**
 * Get access to this singleton object.
 */
CLua *
CLua::Instance ()
{
    static CLua *instance = new CLua ();
    return (instance);
}



/**
 * Constructor - This is private as this class is a singleton.
 */
CLua::CLua ()
{
    /**
     * Create a new Lua object.
     */
    m_lua = luaL_newstate ();


    /**
     * Register the libraries.
     */
    luaopen_base (m_lua);
    luaL_openlibs (m_lua);


    /**
     * Load our bindings.
     */
    InitConfig (m_lua);
    InitMaildir (m_lua);
    InitMessage (m_lua);
    InitMessagePart (m_lua);
    InitScreen (m_lua);
}

/**
 * Destructor.
 */
CLua::~CLua ()
{
    lua_close (m_lua);

}


/**
 * Load the specified Lua file, and evaluate it.
 *
 * Return true on success.  False on error.
 */
bool
CLua::load_file (std::string filename)
{
    int erred = luaL_dofile (m_lua, filename.c_str ());
    if (erred)
	return false;
    else
	return true;
}


/**
 * Evaluate the given string.
 *
 * Return true on success.  False on error.
 */
bool
CLua::execute (std::string lua)
{
    return false;
}
