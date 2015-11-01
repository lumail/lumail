/**
 * message_lua.cc - Bindings for the CMessage C++ object.
 *
 * This file is part of lumail - http://lumail.org/
 *
 * Copyright (c) 2015 by Steve Kemp.  All rights reserved.
 *
 **
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; version 2 dated June, 1991, or (at your
 * option) any later version.
 *
 * On Debian GNU/Linux systems, the complete text of version 2 of the GNU
 * General Public License can be found in `/usr/share/common-licenses/GPL-2'
 */

extern "C"
{
#include <lua.h>
}
#include "message.h"
#include <memory>

extern void push_cmessage(lua_State * l, std::shared_ptr<CMessage> message);
