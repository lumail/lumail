/**
 * lua.h - Singleton interface to an embedded Lua interpreter.
 */

#ifndef _clua_h_
#define _clua_h_ 1

extern "C" {
#include <lua.h>
#include <lauxlib.h>
#include <lualib.h>
}
/**
 * A singleton class holding a Lua intepretter.
 */ class CLua
{
 public:

  /**
   * Get access to the singleton instance.
   */
    static CLua *Instance();

  /**
   * Load and execute a single file of Lua code.
   */
    void loadFile(std::string filename);

  /**
   * Evaluate the given string.
   */
    void execute(std::string lua);

  /**
   * Call a single Lua function, passing no arguments and ignoring the return code.
   */
    bool callFunction(std::string name);

  /**
   * Set a global variable into the Lua environment.
   */
    void setGlobal(std::string name, std::string value);

  /**
   * Get a global variable value from the Lua environment.
   */
     std::string * getGlobal(std::string name);

    /**
     * Execute a function from the global keymap.
     */
    bool onKey(char key);

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

#endif				/* _clua_h_ */
