/*
 * life_view.h - Draw our easter-egg.
 *
 * This file is part of lumail - http://lumail.org/
 *
 * Copyright (c) 2016 by Steve Kemp.  All rights reserved.
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


#pragma once

#include "basic_view.h"

/**
 * This class implements our easter-egg: life-mode.
 *
 */
class CLifeView: public CBasicView
{

public:
    /**
     * Constructor.
     */
    CLifeView();

    /**
     * Destructor.
     */
    ~CLifeView();

    /**
     * Update and re-draw our display.
     */
    void on_idle();
};
