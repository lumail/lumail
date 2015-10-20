/**
 * This is a simple piece of proof of concept for lumail2.
 *
 * The code does three things:
 *
 *    Creates a Lua-callable "Maildir" object.
 *
 *    Creates a Lua-callable "Message" object.
 *
 *    Allows a maildir to return an array of messages.
 *
 * The last point is the reason why this sample was created, because
 * I wanted to be able to write some Lua like this:
 *
 **
 *
 *   dir = Maildir.new( "/home/skx/Maildir/.tldp" )
 *   tmp = dir:messages()
 *
 *   for k,v in pairs(tmp) do
 *      print( k .. " -> " .. v:path() )
 *   end
 *
 **
 *
 * This code seems to work, but I suspect a Lua developer might have
 * words  to say about the style.
 *
 */

extern "C" {
#include <lua.h>
#include <lauxlib.h>
#include <lualib.h>
}

#include <iostream>
#include <gmime/gmime.h>

#include "maildir.h"
#include "message.h"

extern void InitMaildir(lua_State * l);
extern void InitMessage(lua_State * l);



/**
 * The entry point to our code.
 */

int main(int argc, char *argv[])
{
    /**
     * Initi mime.
     */
    g_mime_init(0);

    /**
     * Setup Lua
     */
    lua_State *l = luaL_newstate();
    luaL_openlibs(l);


    /**
     * Setup our objects.
     */
    InitMaildir(l);
    InitMessage(l);

    /**
     * Load the script.
     */
    if (argc > 1)
    {
        int erred = luaL_dofile(l, argv[1]);
        if (erred)
            std::cout << "Lua error: " << luaL_checkstring(l, -1) << std::endl;
    } else
    {
        int erred = luaL_dofile(l, "driver.lua");
        if (erred)
            std::cout << "Lua error: " << luaL_checkstring(l, -1) << std::endl;
    }


    /**
     * All done.
     */
    lua_close(l);
    g_mime_shutdown();
    return 0;
}
