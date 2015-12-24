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


#include <cursesw.h>
#include <panel.h>
#include <string>
#include <unordered_map>
#include <vector>

#include "singleton.h"


/**
 *
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
 * The output function `draw_text_lines` is used to draw lines of text upon
 * the screen, and this function is capable of performing colour output
 * given input containing special markup.
 *
 * Colours are specified via the `$[COLOUR]` prefix, and the specified colour
 * persists until the end of the line.  The following, for example, will draw
 * a line of text with two colours:
 *
 * <code>$[RED]This is red$[YELLOW]This is yellow.</code>
 *
 * Internally the line of text is parsed into segments of text, each of
 * which contains:
 *
 * * The colour to draw
 * * The text to draw.
 *
 * This structure is used to hold that result.
 */
typedef struct _COLOUR_STRING
{
    /**
     * The colour to use for this segment.
     */
    std::string *colour;

    /**
     * The text to draw for this segment.
     */
    std::string *string;

} COLOUR_STRING;



/**
 *
 * This class contains simple functions relating to the screen-handling.
 *
 * Although the class is called `CScreen` it might easily have been called
 * `CLumail`, as most of the logic that is implemented in the C++ part of
 * the codebase is in this class.
 *
 * The class is generally responsible for handling input, updating state,
 * and drawing the display.
 *
 */
class CScreen : public Singleton<CScreen>
{

public:
    /**
     * Constructor.
     */
    CScreen();

    /**
     * Destructor.
     */
    ~CScreen();

public:

    /**
     * Setup the `curses` environment.
     */
    void setup();

    /**
     * Tear down `curses`.
     */
    void teardown();

    /**
     * Run our event loop - which means polling for keyboard input,
     * responding to that received, and redrawing the screen.
     */
    void run_main_loop();

    /**
     * Exit our main event-loop.
     */
    void exit_main_loop();

    /**
     * Return the width of the screen, in columns.
     */
    static int width();

    /**
     * Return the height of the screen, in lines.
     *
     * **NOTE**: This doesn't take any account of the state of the
     * status-panel.
     */
    static int height();

    /**
     * Clear the screen - if refresh_screen is set to true we'll trigger
     * a refresh of the screen via curses.
     */
    void clear(bool refresh_screen = true);

    /**
     * Delay for the given period.
     */
    void sleep(int seconds);

    /**
     * Choose a single item from a small selection.
     *
     * Used by TAB-completion.
     */
    std::string choose_string(std::vector<std::string> choices);

    /**
     * Read a line of input via the status-line.
     *
     * History is handled via our `CHistory` singleton, and faux input
     * may be consumed via our CInputQueue object.
     */
    std::string get_line(std::string prompt, std::string input = "");

    /**
     * Show a message and return only a valid keypress from a given set.
     *
     * Faux input may be consumed via our CInputQueue object.
     */
    std::string prompt_chars(std::string prompt, std::string valid);

    /**
     * Execute a program, resetting the screen first.
     */
    void execute(std::string program);

    /**
     * Is the status-panel visible?
     */
    bool status_panel_visible();

    /**
     * Get the status-panel title.
     */
    std::string status_panel_title();

    /**
     * Get the text contained within the status-panel.
     */
    std::vector < std::string > status_panel_text();

    /**
     * Hide the status-panel.
     */
    void status_panel_hide();

    /**
     * Show the status-panel.
     */
    void status_panel_show();

    /**
     * Append to the status-panel text.
     */
    void status_panel_append(std::string display);

    /**
     * Clear the status-panel text.
     */
    void status_panel_clear();

    /**
     * Set the status-panel title.
     */
    void status_panel_title(std::string new_title);

    /**
     * Toggle the visibility of the status-panel.
     */
    void status_panel_toggle();

    /**
     * Get the height of the status-panel, in lines.
     */
    int status_panel_height();

    /**
     * Set the height of the status-panel - minimum size is six.
     */
    void status_panel_height(int new_size);

    /**
     * Execute a function from the global keymap.
     */
    bool on_keypress(const char *key);

    /**
     * Draw an array of lines to the screen, highlighting the current line.
     *
     * This is used by our view-classes, as a helper.
     *
     * If `simple` is set to true then we display the lines in a  simplified
     * fashion - with no selection, and no smooth-scrolling.
     */
    void draw_text_lines(std::vector<std::string> lines, int selected, int max, bool simple = false);

    /**
     * Draw a single text line, paying attention to our colour strings.
     *
     * **NOTE**: This function is grossly inefficiant, although functional.
     */
    void draw_single_line(int row, int col_offset, std::string text, WINDOW * screen);


private:

    /**
     * Get the colour-pair for the given name.
     */
    int get_colour(std::string name);

    /**
     * Redraw the status-panel.
     */
    void status_panel_draw();

    /**
     * Initialize the status-panel.
     */
    void status_panel_init();

    /**
     * Convert ^I -> TAB, etc.
     */
    const char *lookup_key(int c);

    /**
     * Parse a string into an array of "string + colour" pairs,
     * which will be useful for drawing strings.
     */
    std::vector<COLOUR_STRING *> parse_coloured_string(std::string input);

    /**
     * Given an array of colour parts which might look like this
     *
     * <code>
     *   [ "RED",   "This is in red" ],
     *   [ "YELLOW", "**"]
     * </code>
     *
     * We want to return an updated array that is suitable for drawing column
     * by column such as:
     *
     * <code>
     *  [ "RED", "T" ],
     *  [ "RED", "h" ],
     * ...
     *  [ "YELLOW", *" ],
     *  [ "YELLOW", *" ]
     * </code>
     *
     * This is used to implement horizontal scrolling.
     */
    std::vector<COLOUR_STRING *> coloured_string_scroll(std::vector<COLOUR_STRING *> parts , int offset);

private:

    /**
     * Are we (still) in the event-loop?
     */
    bool m_running;

    /**
     * This map contains a mapping between a given mode-name and the
     * virtual class which implements its display.
     *
     * All display-classes are derived from CViewMode so that we can
     * refer to thim in this generic/interface way.
     */
    std::unordered_map < std::string, CViewMode * >m_views;

    /**
     * A lookup-map of colour-pairs, which are used for drawing
     * coloured text on the screen.
     *
     * See `get_colour` for the accessor used to access this map.
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
