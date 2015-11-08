/*
 * screen.h - Our main application class.
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


/*
 * Only include this header one time.
 */
#pragma once


#include <string>
#include <unordered_map>
#include <vector>

#include "singleton.h"


/*
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

    /*
     * Allow our virtual mode to draw its own display.
     */
    virtual void draw() = 0;

    /*
     * It might be useful to have the virtual modes have
     * an idle function to update things.
     */
    virtual void on_idle() = 0;

};


/*
 * This class contains simple functions relating to the screen-handling.
 */
class CScreen : public Singleton<CScreen>
{

public:
    CScreen();
    ~CScreen();

public:

    /*
     * Setup/Teardown
     */
    void setup();
    void teardown();

    /*
     * Run our event loop.
     */
    void run_main_loop();

    /*
     * Exit our main event-loop
     */
    void exit_main_loop();

    /*
     * Return the width of the screen.
     */
    static int width();

    /*
     * Return the height of the screen.
     */
    static int height();

    /*
     * Clear the screen.
     */
    void clear();

    /*
     * Delay for the given period.
     */
    void sleep(int seconds);

    /*
     * Choose a single item from a small selection.
     *
     * Used by TAB-completion.
     */
    std::string choose_string(std::vector<std::string> choices);

    /*
     * Read a line of input via the status-line.
     */
    std::string get_line(std::string prompt, std::string input = "");

    /*
     * Show a message and return only a valid keypress from a given set.
     */
    std::string prompt_chars(std::string prompt, std::string valid);

    /*
     * Execute a program, resetting the screen first.
     */
    void execute(std::string program);

    /*
     * Is the status-panel visible?
     */
    bool status_panel_visible();

    /*
     * Get the status-panel title.
     */
    std::string status_panel_title();

    /*
     * Get the status-panel text.
     */
    std::vector < std::string > status_panel_text();

    /*
     * Hide the status-panel.
     */
    void status_panel_hide();

    /*
     * Show the status-panel.
     */
    void status_panel_show();

    /*
     * Append to the status-panel text.
     */
    void status_panel_append(std::string display);

    /*
     * Clear the status-panel text.
     */
    void status_panel_clear();

    /*
     * Set the status-panel title.
     */
    void status_panel_title(std::string new_title);

    /*
     * Toggle the visibility of the status-panel.
     */
    void status_panel_toggle();

    /*
     * Get the height of the status-panel.
     */
    int status_panel_height();

    /*
     * Set the height of the status-panel - minimum size is six.
     */
    void status_panel_height(int new_size);

    /*
     * Execute a function from the global keymap.
     */
    bool on_keypress(const char *key);

    /*
     * Get the colour-pair for the given name.
     */
    int get_colour(std::string name);

    /*
     * Draw a list of text, with a current entry highlighted.
     */
    void draw_text_lines(std::vector<std::string> lines, int selected, int max, bool simple = false);

private:
    /*
     * Redraw the status-panel.
     */
    void status_panel_draw();

    /*
     * Initialize the status-panel.
     */
    void status_panel_init();

    /*
     * Convert ^I -> TAB, etc.
     */
    const char *lookup_key(int c);

private:

    /*
     * Are we in the event-loop?
     */
    bool m_running;

    /*
     * This contains the mapping of "global.mode" -> drawing routines.
     */
    std::unordered_map < std::string, CViewMode * >m_views;

    /*
     * Colour-pairs
     */
    std::unordered_map < std::string, int >m_colours;

private:

    /*
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
