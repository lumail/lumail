/**
 * bindings.h - Bindings for all fucntions callable from Lua.
 */

#ifndef _bindings_h
#define _bindings_h  1

#include "lua.h"


int my_function(lua_State * L);
int set_mode(lua_State * L);
int get_mode(lua_State *L);

#endif /* _bindings_h */
