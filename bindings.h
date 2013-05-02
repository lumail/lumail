/**
 * bindings.h - Bindings for all fucntions callable from Lua.
 */

#ifndef _bindings_h
#define _bindings_h  1

#include "lua.h"


/* exit the program */
int exit( lua_State *L);

/* test function */
int my_function(lua_State * L);

/* get/set the global mode: maildir/index/message */
int get_mode(lua_State *L);
int set_mode(lua_State * L);

#endif /* _bindings_h */
