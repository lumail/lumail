
extern "C"
{
#include <lua.h>
#include <lauxlib.h>
#include <lualib.h>
}
#include <iostream>
#include "screen.h"




int
l_CScreen_clear (lua_State * l)
{
    CScreen *foo = CScreen::instance ();
    foo->clear ();
    return 0;
}

int
l_CScreen_height (lua_State * l)
{
    CScreen *foo = CScreen::instance ();
    lua_pushinteger (l, foo->height ());
    return 1;
}

int
l_CScreen_sleep (lua_State * l)
{
    const int delay = luaL_checkinteger (l, 2);
    CScreen *foo = CScreen::instance ();
    foo->sleep (delay);
    return 0;
}

int
l_CScreen_width (lua_State * l)
{
    CScreen *foo = CScreen::instance ();
    lua_pushinteger (l, foo->width ());
    return 1;
}

void
InitScreen (lua_State * l)
{
    luaL_Reg sFooRegs[] = {
	{"clear", l_CScreen_clear},
	{"height", l_CScreen_height},
	{"sleep", l_CScreen_sleep},
	{"width", l_CScreen_width},
	{NULL, NULL}
    };
    luaL_newmetatable (l, "luaL_CScreen");

#if LUA_VERSION_NUM == 501
    luaL_register (l, NULL, sFooRegs);
#elif LUA_VERSION_NUM == 502
    luaL_setfuncs (l, sFooRegs, 0);
#else
#error unsupported Lua version
#endif

    lua_pushvalue (l, -1);
    lua_setfield (l, -1, "__index");
    lua_setglobal (l, "Screen");

}
