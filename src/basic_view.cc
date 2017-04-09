/*
 * basic_view.cc - Basic implementation of screen-drawing.
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


#include "config.h"
#include "lua.h"
#include "basic_view.h"



/*
 * Constructor.
 */
CBasicView::CBasicView()
{
    m_name = "";
    m_function = "";
    m_simple = true;
}


/*
 * Destructor.
 */
CBasicView::~CBasicView()
{
}


/*
 * Get the text to display by calling the specified lua-function.
 */
std::vector<std::string> CBasicView::get_text(std::string function)
{
    /*
     * Call the view-function.
     */
    CLua *lua = CLua::instance();
    std::vector<std::string> result = lua->function2table(function);

    /*
     * Store the number of lines we've retrieved.
     */
    CConfig *config = CConfig::instance();
    config->set(m_name + ".max", result.size());

    return (result);

}


/*
 * This is the virtual function which is called to refresh the display.
 *
 * We invoke the lua function we've been told to use, and setup the
 * offset/current/maximum values to match the generated output.
 */
void CBasicView::draw()
{
    /*
     * If we don't have a function to invoke, to get our
     * display-text we must abort.
     */
    if (m_function.empty())
        return;

    /*
     * Similarly if we don't know what our mode is we can't
     * setup the `$mode.max` variable, so the scrolling won't
     * work.  Simplest to return.
     */
    if (m_name.empty())
        return;

    /*
     * Get the text we're supposed to display, by invoking our
     * lua function.
     */
    std::vector<std::string> txt = get_text(m_function);

    /*
     * No text was output?  Return.
     */
    if (txt.empty())
        return;


    /*
     * Get the currently-selected item, and the size of the lines.
     */
    CConfig *config = CConfig::instance();
    int cur = config->get_integer(m_name + ".current");
    int max = config->get_integer(m_name + ".max");


    /*
     * Ensure our highlight isn't outside reasonable bounds.
     */
    if (cur >= max)
    {
        config->set(m_name + ".current", max - 1, false);
        cur = max - 1;
    }

    if (cur < 0)
    {
        config->set(m_name + ".current", 0, false);
        cur = 0;
    }


    /*
     * Finally draw the text we've received via our screen
     * interface.
     *
     * NOTE: Last argument is "simple" - if true we enable
     * line-wrap, and disable highlighting.  If false we
     * do the opposite.
     */
    CScreen *screen = CScreen::instance();
    screen->draw_text_lines(txt, cur, max, m_simple);

    /**
     * Free the text we have.
     */
    txt.clear();
}


/*
 * Called when things are idle.  NOP.
 */
void CBasicView::on_idle()
{
}


/*
 * Setup our drawing data.
 *
 * `name` is the name of our mode.
 * `function` is the Lua function to invoke to fetch the text to draw.
 * `simple` is whether to draw lines simply, or not.
 */
void CBasicView::set_data(std::string name, std::string function, bool simple)
{
    m_name     = name;
    m_function = function;
    m_simple   = simple;
}
