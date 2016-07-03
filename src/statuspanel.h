/*
 * statuspanel.h - Display/Maintain our status-panel.
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


/*
 * Only include this header one time.
 */
#pragma once

#include "singleton.h"
#include "screen.h"


/**
 * The status-panel is a singleton object which draws text at the
 * foot of our main screen.
 *
 * The handling of this class is pretty naive, but there are two
 * complications which are worth noting:
 *
 *  - When the status-panel is hidden we must destroy the window.
 *    Without this the space is still allocated and curses stops
 *    the display of our main-screen content in the area it used to
 *    occupy.
 *
 *    The symptom is a blank window exactly matching the size of the
 *    old status-panel.
 *
 *
 *  - We keep track of our expected size, and run `wresize` on each
 *    draw operation.  THis handles the terminal resizing, without having
 *    to respond to KEY_RESIZE or SIGWINCH signals.
 *
 */
class CStatusPanel : public Singleton<CStatusPanel>
{
public:
    /**
     * Constructor.
     */
    CStatusPanel();

    /**
     * Destructor
     */
    ~CStatusPanel();

    /**
     * Destroy ourselves, unless already done.
     */
    void cleanup();

    /**
     * Initialize our child window & size.
     */
    void init(int rows);

    /**
     * Return the height of the panel.
     */
    int height();

    /**
     * Show the panel.
     */
    void show();

    /**
     * Hide the panel.
     */
    void hide();

    /**
     * Is the panel hidden?
     */
    bool hidden();

    /**
     * Draw the panel.
     */
    void draw();

    /**
     * Set the panel-title.
     */
    void set_title(std::string new_title);

    /**
     * Get the panel-title.
     */
    std::string get_title();

    /**
     * Remove all text.
     */
    void reset();

    /**
     * Add a line of text to the display.
     */
    void add_text(std::string line);

    /**
     * Get the text we're displaying.
     */
    std::vector<std::string> get_text();

private:


    /**
     * The height of the panel, in number of lines.
     */
    int m_height;

    /**
     * The text the panel contains.
     */
    std::vector < std::string > m_text;

    /**
     * The title of the panel.
     */
    std::string title;

    /**
     * The window.
     */
    WINDOW *g_status_bar_window;

    /**
     * The panel.
     */
    PANEL *g_status_bar;

    /**
     * Are we hidden?
     */
    bool m_hidden;

};
