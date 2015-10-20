
extern "C" {
#include <lua.h>
#include <lauxlib.h>
#include <lualib.h>
}

#include "message_part.h"


CMessagePart *l_CheckCMessagePart(lua_State * l, int n)
{
    return *(CMessagePart **) luaL_checkudata(l, n, "luaL_CMessagePart");
}


int l_CMessagePart_type(lua_State * l)
{
    CMessagePart *foo = l_CheckCMessagePart(l, 1);

    lua_pushstring(l, foo->type().c_str());
    return 1;
}

int l_CMessagePart_destructor(lua_State * l)
{
    CMessagePart *foo = l_CheckCMessagePart(l, 1);
    delete foo;
    return 0;
}

void InitMessagePart(lua_State * l)
{
    luaL_Reg sFooRegs[] = {
        {"type", l_CMessagePart_type},
        {"__gc", l_CMessagePart_destructor},
        {NULL, NULL}
    };
    luaL_newmetatable(l, "luaL_CMessagePart");

#if LUA_VERSION_NUM == 501
    luaL_register(l, NULL, sFooRegs);
#elif LUA_VERSION_NUM == 502
    luaL_setfuncs(l, sFooRegs, 0);
#else
#error unsupported Lua version
#endif

    lua_pushvalue(l, -1);
    lua_setfield(l, -1, "__index");
    lua_setglobal(l, "MessagePart");
}
