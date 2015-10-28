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

#include <algorithm>
#include <sys/ioctl.h>
#include <cursesw.h>
#include <iostream>
#include <fstream>
#include <unistd.h>
#include <panel.h>
#include <string.h>

#include "config.h"
#include "lua.h"
#include "screen.h"

/**
 * Views.
 */
#include "demo_view.h"
#include "history.h"
#include "index_view.h"
#include "lua_view.h"
#include "maildir_view.h"
#include "message_view.h"



/**
 * Data-structure associated with the status-bar.
 *
 * This is used to determine if it is visible or not, as well as the
 * lines of text that are displayed.
 */
typedef struct _PANEL_DATA
{
    int hide;
    std::string title;
    std::vector < std::string > text;
} PANEL_DATA;


/**
 * The status-bar window, panel, & data.
 */
WINDOW *g_status_bar_window;
PANEL *g_status_bar;
PANEL_DATA g_status_bar_data;

#define PANEL_HEIGHT 6


/**
 * Constructor.  Register our view-modes
 */
CScreen::CScreen()
{
    /**
     * Register our view-modes.
     */
    m_views["demo"]    = new CDemoView();
    m_views["index"]   = new CIndexView();
    m_views["lua"]     = new CLuaView();
    m_views["maildir"] = new CMaildirView();
    m_views["message"] = new CMessageView();
}


/**
 * Gain access to our singleton object.
 */
CScreen * CScreen::instance()
{
    static CScreen *instance = new CScreen();
    return (instance);
}

/**
 * Destructor.  NOP.
 */
CScreen::~CScreen()
{
    teardown();
}

/**
 * Run our event loop.
 */
void CScreen::run_main_loop()
{
    /**
     * Timeout on input every half-second.
     */
    timeout(750);

    /**
     * Now we're in our loop.
     */
    m_running = true;

    /**
     * Get the lua-helper.
     */
    CLua *lua = CLua::instance();

    /**
     * Holder for keyboard input.
     */
    int ch;

    while ((m_running) && (ch = getch()))
    {

        /**
         * Clear the screen - note we're using the
         * curses function here, not our method.
         */
        ::clear();


        /**
         * Get the current global mode.
         */
        CConfig *config   = CConfig::instance();
        std::string mode  = config->get_string("global.mode");

        if (mode.empty())
            mode = "maildir";


        /**
         * Get the virtual view class.
         */
        CViewMode *view = m_views[mode];

        /**
         * If the user wanted to quit - do that.
         */
        if (ch == 'Q')
        {
            m_running = false;
            continue;
        }

        /**
         * If the key fetching timed out then call our idle functions.
         */
        if (ch == ERR)
        {
            /**
             * Call the Lua on_idle() function.
             */
            lua->execute("on_idle()");

            /**
             * Call our view-specific on-idle handler.
             */
            view->on_idle();
        }
        else
        {
            /**
             * Fake a string, and call our handler.
             */
            char input[] = { '\0', '\0' };
            input[0] = ch;
            on_keypress(input);
        }

        /**
         * Update the view.
         */
        view->draw();

        /**
         * Update our panel.
         */
        update_panels();
        doupdate();

    }
}


/**
 * Exit our main event-loop
 */
void CScreen::exit_main_loop()
{
    m_running = false;
}


/**
 * Setup the curses/screen.
 */
void CScreen::setup()
{
    /**
     * Setup locale.
     */
    setlocale(LC_CTYPE, "");
    setlocale(LC_ALL, "");

    char e[] = "ESCDELAY=0";
    putenv(e);

    /**
     * Setup ncurses.
     */
    initscr();

    /**
     * Make sure we have colours.
     */
    if (!has_colors() || (start_color() != OK))
    {
        endwin();
        std::cerr << "Missing colour support" << std::endl;
        exit(1);
    }

    /* Initialize curses */
    keypad(stdscr, TRUE);
    crmode();
    noecho();
    curs_set(0);
    timeout(1000);
    use_default_colors();


    /* Initialize all the colors */
    init_pair(1, COLOR_WHITE, -1);
    m_colours[ "white" ] = 1;

    init_pair(2, COLOR_RED, -1);
    m_colours[ "red" ] = 2;

    init_pair(3, COLOR_BLUE, -1);
    m_colours[ "blue" ] = 3;

    init_pair(4, COLOR_GREEN, -1);
    m_colours[ "green" ] = 4;

    init_pair(5, COLOR_CYAN, -1);
    m_colours[ "cyan" ] = 5;

    init_pair(6, COLOR_MAGENTA, -1);
    m_colours[ "magenta" ] = 6;

    init_pair(7, COLOR_YELLOW, -1);
    m_colours[ "yellow" ] = 7;

    init_pair(8, COLOR_BLACK, COLOR_WHITE);
    m_colours[ "black" ] = 8;

    /* Create the status-bar.  Show it */
    init_status_bar();
}


/**
 * Shutdown curses.
 */
void CScreen::teardown()
{
    endwin();
}


/**
 * Clear the whole screen by printing lines of blanks.
 */
void CScreen::clear()
{
    int width = CScreen::width();
    int height = CScreen::height();

    std::string blank = "";

    while ((int) blank.size() < width)
        blank += " ";

    for (int i = 0; i <= height; i++)
    {
        mvprintw(i, 0, "%s", blank.c_str());
    }

    update_panels();
    doupdate();
    refresh();
}


/**
 * Return the height of the screen.
 */
int CScreen::height()
{
    struct winsize w;
    ioctl(0, TIOCGWINSZ, &w);
    return (w.ws_row);
}


/**
 * Delay for the given period.
 */
void CScreen::sleep(int seconds)
{
    ::sleep(seconds);
}

/**
 * Return the width of the screen.
 */
int CScreen::width()
{
    struct winsize w;
    ioctl(0, TIOCGWINSZ, &w);
    return (w.ws_col);
}



/* Create the status-bar */
void CScreen::init_status_bar()
{
    int show = 1;
    int x, y;

    /**
     * Size of panel
     */
    int rows = PANEL_HEIGHT;
    int cols = CScreen::width();

    /**
     * Create the window.
     */
    x = 0;
    y = CScreen::height() - rows;
    g_status_bar_window = newwin(rows, cols, y, x);

    /**
     * Set the content of the status-bar
     */
    g_status_bar_data.title = std::string("Status Panel");
    g_status_bar_data.text.push_back
    ("Lumail v2 UI demo - Toggle panel via 'tab'.  Exit via 'q'.  Eval via ':'.");
    g_status_bar_data.text.push_back("by Steve Kemp");

    /**
     * Refresh the panel display.
     */
    redraw_status_bar();

    /* Attach the panel to the window. */
    g_status_bar = new_panel(g_status_bar_window);
    set_panel_userptr(g_status_bar, &g_status_bar_data);


    if (show)
    {
        show_panel(g_status_bar);
        g_status_bar_data.hide = FALSE;
    }
    else
    {
        hide_panel(g_status_bar);
        g_status_bar_data.hide = TRUE;
    }

}

/**
 * Update the text in the status-bar.
 */
void CScreen::redraw_status_bar()
{
    int width = CScreen::width();

    /**
     * Select white, and draw a box.
     */
    wattron(g_status_bar_window, COLOR_PAIR(1));
    box(g_status_bar_window, 0, 0);
    mvwaddch(g_status_bar_window, 2, 0, ACS_LTEE);
    mvwhline(g_status_bar_window, 2, 1, ACS_HLINE, width - 2);
    mvwaddch(g_status_bar_window, 2, width - 1, ACS_RTEE);

    /**
     * Create a blank string that will ensure shorter/updated
     * titles/lines don't get orphaned on the screen.
     *
     * The width of the screen is the max-length of the string
     * we need to construct - but note that we subtract two:
     *
     *  One for the trailing NULL.
     *  One for the border-character at the end of the line.
     */
    char *blank = (char *) malloc(width);

    for (int i = 0; i < width - 2; i++)
        blank[i] = ' ';

    blank[width - 1] = '\0';

    /**
     * Show the title, and the two lines of text we might have.
     */
    PANEL_DATA x = g_status_bar_data;

    std::string title = x.title;

    if (!title.empty())
    {
        std::string colour = "";

        if (title.at(0) == '$')
        {
            std::size_t start = title.find("[");
            std::size_t end   = title.find("]");

            if ((start != std::string::npos) &&
                    (end != std::string::npos))
            {
                colour = title.substr(start + 1, end - start - 1);
                title  = title.substr(end + 1);
            }
        }

        /**
         * Set the colour, and draw the text.
         */
        if (colour.empty())
            colour = "white";

        wattron(g_status_bar_window, COLOR_PAIR(get_colour(colour)));
        mvwprintw(g_status_bar_window, 1, 1, blank);
        mvwprintw(g_status_bar_window, 1, 1, title.c_str());
    }

    if (x.text.size() > 0)
    {
        /**
         * Starting offset of text-drawing, because we have:
         *
         *  [0] ---
         *  [1] Title goes here
         *  [2] ----
         *
         */
        int line = 3;

        for (std::vector < std::string >::iterator it = x.text.begin();
                it != x.text.end(); it++)
        {
            std::string text   = (*it);
            std::string colour = "";

            if (text.at(0) == '$')
            {
                std::size_t start = text.find("[");
                std::size_t end   = text.find("]");

                if ((start != std::string::npos) &&
                        (end != std::string::npos))
                {
                    colour = text.substr(start + 1, end - start - 1);
                    text   = text.substr(end + 1);
                }
            }

            if (colour.empty())
                colour = "white";

            wattron(g_status_bar_window, COLOR_PAIR(get_colour(colour)));
            mvwprintw(g_status_bar_window, line, 1, blank);
            mvwprintw(g_status_bar_window, line, 1, text.c_str());

            line += 1;
        }
    }

    /**
     * Avoid a leak.
     */
    free(blank);
}


/**
 * Read a line of input via the status-line.
 */
std::string CScreen::get_line()
{
    std::string buffer;

    int old_curs = curs_set(1);
    int pos = 0;
    int x, y;
    int orig_x, orig_y;

    /**
     * Gain access to any past history.
     */
    CHistory *history  = CHistory::instance();
    int history_offset = history->size();

    /**
     * Get the cursor position
     */
    getyx(stdscr, orig_y, orig_x);

    /**
     * Determine where to move the cursor to.  If the panel is visible it'll
     * be above that.
     */
    x = 0;
    y = height() - 1;

    if (g_status_bar_data.hide == FALSE)
    {
        y -= PANEL_HEIGHT;
    }

    mvaddnstr(y, x, ":", 1);

    x = 2;

    while (true)
    {
        int  c;

        mvaddnstr(y, x, buffer.c_str(), buffer.size());

        /**
          * Clear the line- the "-2" comes from the size of the prompt.
          */
        for (int padding = buffer.size(); padding < (width() - 2); padding++)
            printw(" ");

        /**
          * Move the cursor
          */
        move(y, x + pos);

        /**
         * Get some input
         */
        c = getch();

        /**
          * Ropy input-handler.
          */
        if (c == KEY_ENTER || c == '\n' || c == '\r')
        {
            break;
        }
        else if (c == 1)	/* ctrl-a : beginning of line */
        {
            pos = 0;
        }
        else if (c == 5)	/* ctrl-e: end of line */
        {
            pos = buffer.size();
        }
        else if (c == 11)	/* ctrl-k: kill to end of line */
        {
            /**
                 * Kill the buffer from the current position onwards.
                 */
            buffer = buffer.substr(0, pos);
        }
        else if ((c == 2) ||	/* ctrl-b : back char */
                 (c == KEY_LEFT))
        {
            if (pos > 0)
                pos -= 1;
        }
        else if ((c == 6) ||	/* ctrl-f: forward char */
                 (c == KEY_RIGHT))
        {
            if (pos < (int) buffer.size())
                pos += 1;
        }
        else if (c == KEY_UP)
        {
            history_offset -= 1;

            if (history_offset >= 0)
            {
                buffer = history->at(history_offset);
                pos    = buffer.size();
            }
            else
            {
                history_offset = 0;
            }
        }
        else if (c == KEY_DOWN)
        {
            history_offset += 1;

            if (history_offset < history->size())
            {
                buffer = history->at(history_offset);
                pos    = buffer.size();
            }
            else
            {
                history_offset = history->size();
            }
        }
        else if (c == KEY_BACKSPACE)
        {
            if (pos > 0)
            {
                buffer.erase(pos - 1, 1);
                pos -= 1;
            }
        }
        else if (c == 4)	/* ctrl+d */
        {
            /**
             * Remove the character after the point.
             */
            if (pos < (int) buffer.size())
            {
                buffer.erase(pos, 1);
            }
        }
        else if (isprint(c))
        {
            /**
             * Insert the character into the buffer-string.
             */
            buffer.insert(pos, 1, c);
            pos += 1;
        }

    }

    if (old_curs != ERR)
        curs_set(old_curs);

    /**
     * Add the line to the history.
     */
    history->add(buffer);

    return (buffer);
}


void
CScreen::show_status_panel()
{
    show_panel(g_status_bar);
    g_status_bar_data.hide = FALSE;
}

void
CScreen::hide_status_panel()
{
    hide_panel(g_status_bar);
    g_status_bar_data.hide = TRUE;
}

void
CScreen::toggle_status_panel()
{
    if (status_panel_visible())
        hide_status_panel();
    else
        show_status_panel();
}

int CScreen::status_panel_height()
{
    /**
     * TODO - calculate.
     */
    return 6;
}

bool
CScreen::status_panel_visible()
{
    if (g_status_bar_data.hide == FALSE)
        return true;
    else
        return false;
}

void
CScreen::status_panel_title(std::string new_title)
{
    g_status_bar_data.title = new_title;
    redraw_status_bar();
}

std::string CScreen::status_panel_title()
{
    return (g_status_bar_data.title);
}

std::vector < std::string > CScreen::status_panel_text()
{
    return (g_status_bar_data.text);
}

void
CScreen::status_panel_text(std::vector < std::string > new_text)
{
    g_status_bar_data.text = new_text;
    redraw_status_bar();
}


/**
 * Look up the binding for the named keystroke in our keymap(s).
 *
 * If the result is a string then execute it as a function.
 */
bool CScreen::on_keypress(char *key)
{
    /**
     * The result of the lookup.
     */
    char *
    result = NULL;

    /**
     * Get the current global-mode.
     */
    std::string mode = "";

    CConfig *config = CConfig::instance();
    CConfigEntry *ent = config->get("global.mode");

    if ((ent != NULL) && (ent->type == CONFIG_STRING))
        mode = *ent->value.str;

    /**
     * Default mode.
     */
    if (mode.empty())
        mode = "message";

    /**
     * Lookup the keypress in the current-mode-keymap.
     */
    CLua *
    lua = CLua::instance();
    result = lua->get_nested_table("keymap", mode.c_str(), key);


    /**
     * If that failed then lookup the global keymap.
     *
     * This order ensures you can have a "global" keymap, overridden in just one mode.
     */
    if (result == NULL)
        result = lua->get_nested_table("keymap", "global", key);

    /**
     * If one/other of these lookups resulted in success then we're golden.
     */
    if (result != NULL)
        lua->execute(result);

    /**
     * We succeeded if the result wasn't NULL.
     */
    return (result != NULL);
}


int CScreen::get_colour(std::string name)
{
    std::transform(name.begin(), name.end(), name.begin(), ::tolower);
    return (m_colours[name]);
}



/**
 * Draw an array of lines to the screen, highlighting the current line.
 *
 * This is used by our view-classes, as a helper.
 *
 * If `simple` is set to true then we display the lines in a  simplified
 * fashion - with no selection, and no smooth-scrolling.
 *
 */
void CScreen::draw_text_lines(std::vector<std::string> lines, int selected, int max, bool simple)
{
    CScreen *screen = CScreen::instance();
    int height = CScreen::height();

    /**
     * Take off the panel, if visible.
     */
    if (screen->status_panel_visible())
        height -= screen->status_panel_height();

    /**
     * Account for the fact we start from row one not zero.
     */
    height += 1;


    /**
     * If we're in simple-mode we can just draw the lines directly
     * and return - we don't need to worry about the selection-handler
     * or the calculation of the scroll-point.
     */
    if (simple)
    {
        for (int i = 0; i < height; i++)
        {
            if ((i + selected)  < (int)lines.size())
            {
                std::string buf = lines.at(i + selected);

                /**
                 * Look for a colour-string
                 */
                if ((buf.size() > 3) && (buf.at(0) == '$'))
                {
                    std::size_t start = buf.find("[");
                    std::size_t end   = buf.find("]");

                    if ((start != std::string::npos) &&
                            (end != std::string::npos))
                    {
                        std::string colour;
                        colour   = buf.substr(start + 1, end - start - 1);
                        buf    = buf.substr(end + 1);

                        wattron(stdscr, COLOR_PAIR(screen->get_colour(colour)));
                    }
                }

                /**
                 * Ensure we draw a complete line.
                 */
                while ((int)buf.length() < CScreen::width())
                    buf += " ";

                /**
                 * Ensure the line isn't too long, so we don't wrap around.
                 */
                if ((int)buf.length() >  CScreen::width())
                    buf = buf.substr(0, CScreen::width() - 1);

                /**
                 *  Draw the line, and reset any changed-colour.
                 */
                mvprintw(i, 0, "%s", buf.c_str());
                wattron(stdscr, COLOR_PAIR(screen->get_colour("white")));
            }
        }

        return;
    }


    /**
     * This is complex/smooth-scrolling mode.
     *
     * We'll draw a highlighted bar, and that'll move "nicely".
     */


    int middle = (height) / 2;
    int rowToHighlight = 0;
    vectorPosition topBottomOrMiddle = NONE;

    /**
     * default to TOP if our list is shorter then the screen height
     */
    if (selected < middle || max <= height)
    {
        topBottomOrMiddle = TOP;
        rowToHighlight = selected;
        /**
         * if height is uneven we have to switch to the BOTTOM case on row earlier
         */
    }
    else if ((max - selected <= middle) || (height % 2 == 1 && max - selected <= middle + 1))
    {
        topBottomOrMiddle = BOTTOM;
        rowToHighlight =  height - max + selected - 1 ;
    }
    else
    {
        topBottomOrMiddle = MIDDLE;
        rowToHighlight = middle;
    }


    for (int row = 0; row < height; row++)
    {
        /**
         * The current object.
         */
        int mailIndex = max;

        if (topBottomOrMiddle == TOP)
        {
            /**
             * we start at the top of the list so just use row
             */
            mailIndex = row;
        }
        else if (topBottomOrMiddle == BOTTOM)
        {
            /**
             * when we reached the end of the list mailIndex can maximally be
             * count-1, that this is given can easily be shown
             * row:=height-2 -> count-height+row+1 = count-height+height-2+1 = count-1
             */
            mailIndex = max - height + row + 1;
        }
        else if (topBottomOrMiddle == MIDDLE)
        {
            mailIndex = row + selected - middle;
        }


        std::string buf;

        if ((mailIndex < max) && (mailIndex < (int)lines.size()))
            buf = lines.at(mailIndex);

        if (buf.empty())
            continue;

        if (row == rowToHighlight)
            wattron(stdscr, A_REVERSE | A_STANDOUT);
        else
            wattroff(stdscr, A_REVERSE | A_STANDOUT);

        /**
         * Look for a colour-string
         */
        if ((buf.size() > 3) && (buf.at(0) == '$'))
        {
            std::size_t start = buf.find("[");
            std::size_t end   = buf.find("]");

            if ((start != std::string::npos) &&
                    (end != std::string::npos))
            {
                std::string colour;
                colour   = buf.substr(start + 1, end - start - 1);
                buf    = buf.substr(end + 1);

                wattron(stdscr, COLOR_PAIR(screen->get_colour(colour)));
            }
        }

        /**
         * Ensure we draw a complete line - so that we cover
         * any old text - and make sure that our highlight covers a complete
         * line.
         */
        while ((int)buf.length() < CScreen::width())
            buf += " ";

        /**
         * Ensure the line isn't too long, so we don't
         * wrap around.
         */
        if ((int)buf.length() >  CScreen::width())
            buf = buf.substr(0, CScreen::width() - 1);

        /**
         * Show the line, and reset the colours to known-good.
         */
        mvprintw(row, 0, "%s", buf.c_str());
        wattron(stdscr, COLOR_PAIR(screen->get_colour("white")));
    }

    /**
     * Ensure we turn off the attribute on the last line - so that
     * any blank lines are "normal".
     */
    wattroff(stdscr, A_REVERSE | A_STANDOUT);
    wattron(stdscr, COLOR_PAIR(screen->get_colour("white")));

}
