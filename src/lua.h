
#ifndef _LUA_H_
#define _LUA_H_ 1

extern "C"
{
#include <lua.h>
#include <lauxlib.h>
#include <lualib.h>
}

#include <string>


/**
 * A singleton class holding a Lua interpreter.
 *
 */
class CLua
{
  private:
    CLua ();
    ~CLua ();

  public:

    /**
     * Get access to this singleton instance.
     */
    static CLua *Instance ();

    /**
     * Load the specified Lua file, and evaluate it.
     *
     * Return true on success.  False on error.
     */
    bool load_file (std::string filename);

    /**
     * Evaluate the given string.
     *
     * Return true on success.  False on error.
     */
    bool execute (std::string lua);

    /**
     * Lookup a key in a nested table structure - used for keyboard lookups.
     */
    char *get_nested_table (std::string table, const char *key,
			    const char *subkey);


  private:

    /**
     * The handle to the Lua interpreter.
     */
         lua_State * m_lua;

};

#endif
