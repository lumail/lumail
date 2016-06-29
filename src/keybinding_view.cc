/*
 * keybinding_view.cc - Show available keybindings.
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


#include "keybinding_view.h"


/*
 * Ensure we're registered as a valid view mode.
 */
REGISTER_VIEW_MODE(keybinding, CKeyBindingView)


/*
 * Constructor.
 */
CKeyBindingView::CKeyBindingView()
{
    set_data("keybinding", "keybinding_view", true);
}


/*
 * Destructor.
 */
CKeyBindingView::~CKeyBindingView()
{
}
