/*
 * life_view.h - Draw our easter-egg.
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


#include "lua.h"
#include "life_view.h"



/*
 * Ensure we're registered as a valid view mode.
 */
REGISTER_VIEW_MODE(life, CLifeView)


/*
 * Constructor.
 */
CLifeView::CLifeView()
{
    set_data("life", "life_view", true);
}


/*
 * Destructor.
 */
CLifeView::~CLifeView()
{
}


/*
 * Update and re-draw our display.
 */
void CLifeView::on_idle()
{
    CLua *lua = CLua::instance();

    lua->execute( "life:print_matrix()" );
    lua->execute( "life:next_gen()" );
}
