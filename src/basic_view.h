/*
 * basic_view.h - Basic implementation of screen-drawing.
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

#include <vector>
#include <string>
#include "screen.h"


/**
 * This class implements the drawing for all of our stock views.
 *
 * It is built upon the observation that all the (current) modes are
 * largely identical:
 *
 *  1.  A lua function is called to get some text to display.
 *
 *  2.  The text is drawn, in either simple or complex modes.
 *
 *  3.  The `on_idle` function does nothing.
 *
 * The "simple" vs "complex" drawing just means that in simple-modes
 * we draw the text, and in complex-modes we draw a highlight over
 * the current line.
 *
 * Simple modes also allow line-wrapping, wheras the complex ones do not
 * (because that would involve breaking the assumption that a single line
 * can be highlighted, and that line is literally one line).
 *
 */
class CBasicView: public CViewMode
{

public:
    /**
     * Constructor.
     */
    CBasicView();

    /**
     * Destructor.
     */
    ~CBasicView();

    /**
     * This is the virtual function which is called to refresh the display.
     *
     * We invoke the Lua callback function to get our text, and then
     * display it.
     */
    void draw();

    /**
     * Called when things are idle.  NOP
     */
    void on_idle();

    /**
     * Setup our drawing data.
     *
     * `name` is the name of our mode.
     * `function` is the Lua function to invoke to fetch the text to draw.
     * `simple` is whether to draw lines simply, or not.
     */
    void set_data(std::string name, std::string function, bool simple);

private:

    /**
     * Get the display text by calling the specified lua-function.
     */
    std::vector<std::string> get_text(std::string function);

    /**
     * The name of this mode.  e.g. "lua", "index", etc.
     */
    std::string m_name;

    /**
     * The lua-function that will generate our output.
     */
    std::string m_function;

    /**
     * Does this mode use simple-scrolling?
     */
    bool m_simple;
};
