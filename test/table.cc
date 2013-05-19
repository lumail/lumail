/**
 * Read from a table in C++
 */
#include <cstdlib>
#include <iostream>
#include <fstream>

extern "C" {
  #include <lua.h>
  #include <lauxlib.h>
  #include <lualib.h>
}


/**
 * Lua Handle
 */
lua_State *m_lua;

/**
 * get the value from a table.
 */
const char *get_table( const char *name, const char *mode, const char *keypress )
{
    const char *result = NULL;

    lua_getglobal(m_lua, name );
    if (lua_isnil (m_lua, -1 ) ) {
        fprintf(stderr, "FAILED TO FIND global: %s\n", name );
        return result;
    }

    lua_pushstring(m_lua, mode );
    lua_gettable(m_lua, -2);
    if (! lua_istable(m_lua, -1 ) )
    {
        fprintf(stderr, "Not a sub-table: %s[%s]\n", name, mode );
        return result;
    }

    lua_pushstring(m_lua, keypress);
    lua_gettable(m_lua, -2);
    if (lua_isnil (m_lua, -1 ) ) {
        fprintf(stderr, "FAILED TO FIND keypress: %s\n", keypress );
        return result;
    }

    if (lua_isstring(m_lua, -1))
        result = (char *)lua_tostring(m_lua, -1);
    return result;
}

/**
 * Do this thing.
 */
int main( int argc, char *argv[] )
{
  m_lua = lua_open();
  luaopen_base(m_lua);
  luaL_openlibs(m_lua);

  /**
   * Call our Lua file.
   */
  int s = luaL_loadfile(m_lua, "table.lua");
  if ( s==0 ) {
    s = lua_pcall(m_lua, 0, LUA_MULTRET, 0);
  }
  else {
    fprintf(stderr, "Error calling lua: %s\n", lua_tostring(m_lua, -1) );
  }

  /**
   * Now get some tables.
   */
  const char *one = get_table( "keymap" , "global", "RETURN" );
  if ( one != NULL )
      std::cout << one << std::endl;


  lua_close(m_lua);
  return 0;

}


