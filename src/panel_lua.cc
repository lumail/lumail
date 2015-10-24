
extern "C"
{
#include <lua.h>
#include <lauxlib.h>
#include <lualib.h>
}

#include "screen.h"



int
l_CPanel_hide (lua_State * l)
{
    CScreen *screen = CScreen::instance ();
    screen->hide_status_panel ();
    return 0;
}

int
l_CPanel_show (lua_State * l)
{
    CScreen *screen = CScreen::instance ();
    screen->show_status_panel ();
    return 0;
}


int
l_CPanel_title (lua_State * l)
{
    CScreen *screen = CScreen::instance ();

    const char *str = lua_tostring (l, 2);
    if (str != NULL)
    {
	screen->status_panel_title (str);
	return 0;
    }
    else
    {
	std::string existing = screen->status_panel_title ();
	lua_pushstring (l, existing.c_str ());
	return 1;
    }
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
	{"hide", l_CPanel_hide},
	{"show", l_CPanel_show},
	{"title", l_CPanel_title},
	{"toggle", l_CPanel_toggle},
	{"visible", l_CPanel_visible},
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
