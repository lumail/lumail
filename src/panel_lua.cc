
extern "C"
{
#include <lua.h>
#include <lauxlib.h>
#include <lualib.h>
}

#include "screen.h"



int
l_CPanel_show (lua_State * l)
{
    CScreen *screen = CScreen::instance ();
    screen->show_status_panel ();
    return 0;
}

int
l_CPanel_hide (lua_State * l)
{
    CScreen *screen = CScreen::instance ();
    screen->hide_status_panel ();
    return 0;
}

int
l_CPanel_toggle (lua_State * l)
{
    CScreen *screen = CScreen::instance ();
    screen->toggle_status_panel ();
    return 0;
}

int
l_CPanel_visible (lua_State * l)
{
    CScreen *screen = CScreen::instance ();
    if (screen->status_panel_visible ())
	lua_pushboolean (l, 1);
    else
	lua_pushboolean (l, 0);
    return 1;
}

void
InitPanel (lua_State * l)
{
    luaL_Reg sFooRegs[] = {
	{"visible", l_CPanel_visible},
	{"show", l_CPanel_show},
	{"hide", l_CPanel_hide},
	{"toggle", l_CPanel_toggle},
	{NULL, NULL}
    };
    luaL_newmetatable (l, "luaL_CPanel");

#if LUA_VERSION_NUM == 501
    luaL_register (l, NULL, sFooRegs);
#elif LUA_VERSION_NUM == 502
    luaL_setfuncs (l, sFooRegs, 0);
#else
#error unsupported Lua version
#endif

    lua_pushvalue (l, -1);
    lua_setfield (l, -1, "__index");
    lua_setglobal (l, "Panel");
}
