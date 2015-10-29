/**
 * $FILENAME - $TITLE
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


/**
 * Only include this header one time.
 */
#pragma once


#include <string>
#include <unordered_map>
#include <vector>


/**
 * This is the base-class for our virtual views.
 *
 * In the future this will be more advanced, based on the observation
 * that all of our modes contain some magic relating to "scrolling".
 *
 * With that in mind we'll allow derived classes to implement "min", "max", and
 * "current_offset".
 *
 * We'll have a "next" and "prev" class in the base which implements movement
 * within the defined ranges.
 *
 * Or something like that anyway :)
 *
 */
class CViewMode
{
public:

    /**
     * Allow our virtual mode to draw its own display.
     */
    virtual void draw() = 0;

    /**
     * It might be useful to have the virtual modes have
     * an idle function to update things.
     */
    virtual void on_idle() = 0;

};


/**
 * This class contains simple functions relating to the screen-handling.
 */
class CScreen
{

private:
    CScreen();

    /**
     * Destructor.  NOP.
     */
    ~CScreen();

public:

    /**
     * Instance accessor - this is a singleton.
     */
    static CScreen *instance();


    /**
     * Setup/Teardown
     */
    void setup();
    void teardown();

    /**
     * Run our event loop.
     */
    void run_main_loop();

    /**
     * Exit our main event-loop
     */
    void exit_main_loop();

    /**
     * Return the width of the screen.
     */
    static int width();

    /**
     * Return the height of the screen.
     */
    static int height();

    /**
     * Clear the screen.
     */
    void clear();

    /**
     * Delay for the given period.
     */
    void sleep(int seconds);

    /**
     * Read a line of input via the status-line.
     */
    std::string get_line();

    /**
     * Panel-related functions.
     */
    bool status_panel_visible();
    std::string status_panel_title();
    std::vector < std::string > status_panel_text();
    void hide_status_panel();
    void show_status_panel();
    void status_panel_text(std::vector < std::string >);
    void status_panel_title(std::string new_title);
    void toggle_status_panel();
    int status_panel_height();

    /**
     * Execute a function from the global keymap.
     */
    bool on_keypress(const char *key);

    /**
     * Get the colour-pair for the given name.
     */
    int get_colour(std::string name);


    /**
     * Draw a list of text, with a current entry highlighted.
     */
    void draw_text_lines(std::vector<std::string> lines, int selected, int max, bool simple = false);

private:
    void redraw_status_bar();
    void init_status_bar();


    /**
     * Convert ^I -> TAB, etc.
     */
    const char *lookup_key(int c);

private:

    /**
     * Are we in the event-loop?
     */
    bool m_running;

    /**
     * This contains the mapping of "global.mode" -> drawing routines.
     */
    std::unordered_map < std::string, CViewMode * >m_views;

    /**
     * Colour-pairs
     */
    std::unordered_map < std::string, int >m_colours;

private:

    /**
     * The segment of the screen the highlighted row is within.
     *
     * Used by `draw_text_lines`.
     */
    enum vectorPosition
    {
        TOP,
        MIDDLE,
        BOTTOM,
        NONE
    };
};
