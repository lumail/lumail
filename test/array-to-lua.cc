/**
 * Create and populate a table (read: array) in C++ which is visible to Lua.
 */

extern "C" {
  #include <lua.h>
  #include <lauxlib.h>
  #include <lualib.h>
}


/**
 * Do this thing.
 */
int main( int argc, char *argv[] )
{
  lua_State *m_lua;

  m_lua = lua_open();
  luaopen_base(m_lua);
  luaL_openlibs(m_lua);

  /**
   * Create the table.
   */
  lua_newtable(m_lua);

  /**
   * Add one entry.
   */
  lua_pushnumber(m_lua,1);
  lua_pushstring(m_lua,"one");
  lua_settable(m_lua,-3);

  /**
   * Add a second entry.
   */
  lua_pushnumber(m_lua,2);
  lua_pushstring(m_lua,"two");
  lua_settable(m_lua,-3);

  /**
   * Make the table availble with the name "array".
   */
  lua_setglobal(m_lua, "array");

  /**
   * Call the file.
   */
  int s = luaL_loadfile(m_lua, "array-to-lua.lua");
  if ( s==0 ) {
    s = lua_pcall(m_lua, 0, LUA_MULTRET, 0);
  }
  else {
    fprintf(stderr, "Error calling lua: %s\n", lua_tostring(m_lua, -1) );
  }

  lua_close(m_lua);
  return 0;

}


