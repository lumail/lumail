/*
 * screen.cc - Our main object.
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
#include <fstream>
#include <iostream>
#include <string.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include <pcrecpp.h>

#include "attachment_view.h"
#include "config.h"
#include "history.h"
#include "index_view.h"
#include "lua.h"
#include "lua_view.h"
#include "maildir_view.h"
#include "message_view.h"
#include "screen.h"



/*
 * Used for `draw_single_line` function.
 */
#ifndef ARRAY_SIZE
#  define ARRAY_SIZE( array ) sizeof( array ) / sizeof( array[0] )
#endif


/**
 * Data-structure associated with the status-bar.
 *
 * This is used to determine if it is visible or not, as well as the
 * lines of text that are displayed.
 */
typedef struct _PANEL_DATA
{
    /**
       * Is the panel hidden?
       */
    bool hidden;

    /**
       * The total height of the panel, in number of lines.
       */
    int height;

    /**
       * The title of the panel.
       */
    std::string title;

    /**
       * The text the panel contains.
       */
    std::vector < std::string > text;
} PANEL_DATA;


/*
 * The status-bar window, panel, & data.
 */
WINDOW *g_status_bar_window;
PANEL *g_status_bar;
PANEL_DATA g_status_bar_data;




/*
 * Constructor.
 */
CScreen::CScreen()
{
    /*
     * Register our view-modes.
     */
    m_views["attachment"] = new CAttachmentView();
    m_views["index"]      = new CIndexView();
    m_views["lua"]        = new CLuaView();
    m_views["maildir"]    = new CMaildirView();
    m_views["message"]    = new CMessageView();
}



/*
 * Destructor.  NOP.
 */
CScreen::~CScreen()
{
    teardown();
}

/*
 * Run our event loop.
 */
void CScreen::run_main_loop()
{
    /*
     * Get our timeout period, and set it.
     */
    CConfig *config = CConfig::instance();
    int tout = config->get_integer("global.timeout");

    if (tout == 0)
        tout = 750;

    timeout(tout);

    /*
     * Now we're in our loop.
     */
    m_running = true;

    /*
     * Get the lua-helper.
     */
    CLua *lua = CLua::instance();

    /*
     * Holder for keyboard input.
     */
    int ch;

    while ((m_running) && (ch = getch()))
    {

        /*
         * Clear the screen - note we're using the
         * curses function here, not our method.
         */
        ::clear();


        /*
         * Get the current global mode.
         */
        CConfig *config   = CConfig::instance();
        std::string mode  = config->get_string("global.mode");

        if (mode.empty())
            mode = "maildir";


        /*
         * Get the virtual view class.
         */
        CViewMode *view = m_views[mode];

        /*
         * If the user wanted to quit - do that.
         */
        if (ch == 'Q')
        {
            m_running = false;
            continue;
        }

        /*
         * If the key fetching timed out then call our idle functions.
         */
        if (ch == ERR)
        {
            /*
             * Call the Lua on_idle() function.
             */
            lua->execute("on_idle()");

            /*
             * Call our view-specific on-idle handler.
             */
            view->on_idle();
        }
        else
        {
            /*
             * Convert the key-press to a key-name, which means that
             * "down" will be "KEY_DOWN", for example.
             */
            const char *key = lookup_key(ch);

            if (key != NULL)
                on_keypress(key);
        }

        /*
         * Update the view.
         */
        view->draw();

        /*
         * Update our panel.
         */
        update_panels();
        doupdate();

    }
}


/*
 * Exit our main event-loop
 */
void CScreen::exit_main_loop()
{
    m_running = false;
}


/*
 * Execute a command via `system`.
 */
void CScreen::execute(std::string prog)
{
    int result __attribute__((unused));

    /*
     * Save the current state of the TTY
     */
    refresh();
    def_prog_mode();
    endwin();

    /* Run the command */
    result = system(prog.c_str());

    /*
     * Reset + redraw
     */
    reset_prog_mode();
    refresh();
}

/*
 * Convert "^I" -> "TAB", etc.
 */
const char *CScreen::lookup_key(int c)
{
    if (c == '\n')
        return ("ENTER");

    if (c == '\t')
        return ("TAB");

    if (c == ' ')
        return ("SPACE");

    return (keyname(c));
}


/*
 * Setup the curses/screen.
 */
void CScreen::setup()
{
    /*
     * Setup locale.
     */
    setlocale(LC_CTYPE, "");
    setlocale(LC_ALL, "");

    char e[] = "ESCDELAY=0";
    putenv(e);

    /*
     * Setup ncurses.
     */
    initscr();

    /*
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

    /*
     * Get our timeout period, and set it.
     */
    CConfig *config = CConfig::instance();
    int tout = config->get_integer("global.timeout");

    if (tout == 0)
        tout = 750;

    timeout(tout);
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
    status_panel_init();
}


/*
 * Shutdown curses.
 */
void CScreen::teardown()
{
    endwin();
}


/*
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


/*
 * Return the height of the screen.
 */
int CScreen::height()
{
    struct winsize w;
    ioctl(0, TIOCGWINSZ, &w);
    return (w.ws_row);
}


/*
 * Delay for the given period.
 */
void CScreen::sleep(int seconds)
{
    ::sleep(seconds);
}

/*
 * Return the width of the screen.
 */
int CScreen::width()
{
    struct winsize w;
    ioctl(0, TIOCGWINSZ, &w);
    return (w.ws_col);
}



/*
 *  Create the status-panel.
 */
void CScreen::status_panel_init()
{
    int show = 1;
    int x, y;

    /*
     * Size of panel
     */
    int rows = 6;
    int cols = CScreen::width();

    /*
     * Create the window.
     */
    x = 0;
    y = CScreen::height() - rows;
    g_status_bar_window = newwin(rows, cols, y, x);

    /*
     * Set the content of the status-bar
     */
    g_status_bar_data.height = rows;
    g_status_bar_data.title = std::string("Status Panel");
    g_status_bar_data.text.push_back
    ("Lumail v2 - Toggle panel via 'TAB'.  Exit via 'Q'.  Eval via ':'.");
    g_status_bar_data.text.push_back("by Steve Kemp");

    /*
     * Refresh the panel display.
     */
    status_panel_draw();

    /* Attach the panel to the window. */
    g_status_bar = new_panel(g_status_bar_window);
    set_panel_userptr(g_status_bar, &g_status_bar_data);


    if (show)
    {
        show_panel(g_status_bar);
        g_status_bar_data.hidden = false;
    }
    else
    {
        hide_panel(g_status_bar);
        g_status_bar_data.hidden = true;
    }

}

/*
 * Update the text in the status-bar.
 */
void CScreen::status_panel_draw()
{
    int width = CScreen::width();

    /*
     * Show the title, and the last two lines of the text.
     */
    PANEL_DATA x = g_status_bar_data;

    if (! x.title.empty())
    {
        draw_single_line(1, 1, x.title, g_status_bar_window);
    }


    if (x.text.size() > 0)
    {
        int height = g_status_bar_data.height;

        /*
         * Reverse the lines of text, and draw until we've exceeded
         * our height.
         */
        std::vector<std::string> tmp = x.text;
        std::reverse(tmp.begin(), tmp.end());

        int i = 0;

        while (i < (height - 3 - 1))
        {
            std::string text;

            if (i < (int)tmp.size())
                text = tmp.at(i);
            else
                text = "";

            draw_single_line((height - 2 - i), 1, text, g_status_bar_window);
            i++;
        }
    }

    /*
     * Select white, and draw a box.
     */
    wattron(g_status_bar_window, COLOR_PAIR(1));
    box(g_status_bar_window, 0, 0);
    mvwaddch(g_status_bar_window, 2, 0, ACS_LTEE);
    mvwhline(g_status_bar_window, 2, 1, ACS_HLINE, width - 2);
    mvwaddch(g_status_bar_window, 2, width - 1, ACS_RTEE);


}


/*
 * Choose a single item from a small selection.
 *
 * (This is used to resolve ambiguity in TAB-completion.)
 */
std::string CScreen::choose_string(std::vector<std::string> choices)
{
    /*
     * We don't need to resolve ambiguity unless there is more than
     * one choice to choose from.
     */
    assert(choices.size() > 0);

    /*
     * Find longest/widest entry.
     */
    size_t max = 0;

    for (std::string choice : choices)
    {
        if (choice.size() > max)
            max = choice.size();
    }

    /*
     * Get the dimensions.
     */
    int height = CScreen::height() - 4;
    int width = CScreen::width() - 4;
    size_t cols = 1;

    WINDOW *childwin = newwin(height, width, 2, 2);
    box(childwin, 0, 0);

    /*
     * How many columns to draw?
     */
    for (int i = 1; i < 12; i++)
    {
        if (max < (size_t(width) / i))
            cols = i;
    }

    /*
     * We'll be careful to not draw more columns than we have items.
     */
    if (cols > choices.size())
        cols = choices.size();


    int selected  = 0;
    bool done     = false;
    int col_width = width / cols;

    timeout(0);

    while (!done)
    {
        refresh();

        int count = 0;

        /*
         * Drawing of each item.
         */
        int x = 0;
        int y = 2;

        for (std::string choice : choices)
        {

            /*
             * Calculate the column.
             */
            x = 2 + ((count % cols) * col_width);
            y = 1 + (count / cols);


            if (count == selected)
                wattron(childwin, A_UNDERLINE | A_STANDOUT);
            else
                wattroff(childwin, A_UNDERLINE | A_STANDOUT);

            mvwaddstr(childwin, y, x,  choice.c_str());
            count += 1;
        }

        wrefresh(childwin);

        int c = getch();

        if (c == '\n')
            done = true;

        if (c == 27)
        {
            delwin(childwin);
            ::clear();
            /*
             * Get our timeout period, and set it.
             */
            CConfig *config = CConfig::instance();
            int tout = config->get_integer("global.timeout");

            if (tout == 0)
                tout = 750;

            timeout(tout);
            return "";
        }

        if (c == '\t')
        {
            selected += 1;

            if (selected >= (int)choices.size())
                selected = 0;
        }

        if (c == KEY_RIGHT)
        {
            selected += 1;

            if (selected >= (int)choices.size())
                selected = 0;
        }

        if (c == KEY_LEFT)
        {
            selected -= 1;

            if (selected < 0)
                selected = (int)(choices.size() - 1);

        }
    }

    delwin(childwin);
    ::clear();
    /*
     * Get our timeout period, and set it.
     */
    CConfig *config = CConfig::instance();
    int tout = config->get_integer("global.timeout");

    if (tout == 0)
        tout = 750;

    timeout(tout);
    return (choices.at(selected));
}


/*
 * Read a line of input via the status-line.
 */
std::string CScreen::get_line(std::string prompt, std::string input)
{
    std::string buffer;


    int old_curs = curs_set(1);
    int pos = 0;
    int x, y;
    int orig_x, orig_y;

    if (! input.empty())
    {
        buffer = input;
        pos = input.size();
    }

    /*
     * Gain access to any past history.
     */
    CHistory *history  = CHistory::instance();
    int history_offset = history->size();

    /*
     * Get the cursor position
     */
    getyx(stdscr, orig_y, orig_x);

    /*
     * Determine where to move the cursor to.  If the panel is visible it'll
     * be above that.
     */
    x = 0;
    y = height() - 1;

    if (!g_status_bar_data.hidden)
    {
        y -= g_status_bar_data.height;
    }

    /*
     * Draw the prompt, and make sure we place the cursor at a suitable spot.
     */
    mvaddnstr(y, x, prompt.c_str(), prompt.length());
    x += prompt.length();

    /*
     * Get the mode so we can update the display mid-input.
     */
    CConfig *config   = CConfig::instance();
    std::string mode  = config->get_string("global.mode");

    if (mode.empty())
        mode = "maildir";

    CViewMode *view = m_views[mode];

    /*
     * We'll call our idle function too.
     */
    CLua *lua = CLua::instance();

    while (true)
    {
        int  c;

        /*
         * Redraw the main display.
         */
        if (view)
        {
            view->draw();
            update_panels();
        }

        /*
         * Call the Lua on_idle() function.
         */
        lua->execute("on_idle()");

        mvaddnstr(y, 0, prompt.c_str(), prompt.length());
        mvaddnstr(y, x, buffer.c_str(), buffer.size());

        /*
          * Clear the line- the "-2" comes from the size of the prompt.
          */
        for (int padding = buffer.size(); padding < (width() - 1 - (int)prompt.length()); padding++)
            printw(" ");

        /*
          * Move the cursor
          */
        move(y, x + pos);

        /*
         * Get some input
         */
        c = getch();

        /*
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
            /*
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
            /*
             * Remove the character after the point.
             */
            if (pos < (int) buffer.size())
            {
                buffer.erase(pos, 1);
            }
        }
        else if ((c == '\t') && (! buffer.empty()))      /* TAB-completion */
        {
            /*
             * We're going to find the token to complete against
             * by searching backwards for a position to start from.
             *
             * This string comes from lua, and includes things like: ( " ' space
             *
             */
            size_t toke = buffer.find_last_of("(\"' ", pos);

            std::string prefix = "";
            std::string token  = buffer;

            /*
             * If we found one of the split-characters then we have
             * a token to complete, and the prefix to ignore.
             *
             * If we didn't then the prefix is empty, and the buffer is
             * the token; i.e. we're completing the sole token on the line.
             *
             * NOTE:  This implies you cannot complete in the middle of a line.
             * Just at the end.  Or start.
             *
             */
            if (toke != std::string::npos)
            {
                prefix = buffer.substr(0, toke + 1);
                token = token.substr(toke + 1);
            }


            /*
             * The token length - because we want to update the cursor position, post-completion.
             */
            int toke_len = token.size();

            /*
             * Get the completions.
             */
            CLua *lua = CLua::instance();
            std::vector<std::string> matches = lua->get_completions(token);

            if (matches.size() == 0)
            {
                /*
                 * No completion possible.
                 */
                beep();
            }
            else
            {
                /*
                 * Single completion == match.
                 */
                if (matches.size() == 1)
                {
                    buffer = prefix + matches.at(0).c_str();
                    pos += (matches.at(0).size() - toke_len);
                }
                else
                {
                    /*
                     * Disable echoing before showing the menu.
                     */
                    noecho();
                    curs_set(0);

                    /*
                     * Prompt for clarification in the multiple-matches.
                     */
                    std::string choice = choose_string(matches);

                    /*
                     * Reset the cursor.
                     */
                    curs_set(1);
                    echo();

                    /*
                     * If the user did make a specific choice, then use it.
                     */
                    if (! choice.empty())
                    {
                        buffer = prefix + choice.c_str();
                        pos += (choice.size() - toke_len);
                    }
                }
            }
        }
        else if (isprint(c))
        {
            /*
             * Insert the character into the buffer-string.
             */
            buffer.insert(pos, 1, c);
            pos += 1;
        }

    }

    if (old_curs != ERR)
        curs_set(old_curs);

    /*
     * Add the line to the history.
     */
    history->add(buffer);

    return (buffer);
}


/*
 * Show a message and return only a valid keypress from a given set.
 */
std::string CScreen::prompt_chars(std::string prompt, std::string valid)
{
    int orig_x, x;
    int orig_y, y;

    /*
     * Get the cursor position
     */
    getyx(stdscr, orig_y, orig_x);

    /*
     * Ensure we draw a complete line.
     */
    while ((int)prompt.length() < CScreen::width())
        prompt += " ";

    /*
     * Determine where to move the cursor to.  If the panel is visible it'll
     * be above that.
     */
    x = 0;
    y = height() - 1;

    if (!g_status_bar_data.hidden)
    {
        y -= g_status_bar_data.height;
    }


    /*
     * Get the mode so we can update the display mid-input.
     */
    CConfig *config   = CConfig::instance();
    std::string mode  = config->get_string("global.mode");

    if (mode.empty())
        mode = "maildir";

    CViewMode *view = m_views[mode];

    /*
     * We'll call our idle function too.
     */
    CLua *lua = CLua::instance();



    while (true)
    {
        int  c;

        /*
         * Redraw the main display.
         */
        if (view)
        {
            view->draw();
            update_panels();
        }

        /*
         * Call the Lua on_idle() function.
         */
        lua->execute("on_idle()");

        mvaddnstr(y, x, prompt.c_str(), prompt.length());

        c = getch();

        for (unsigned int i = 0; i < valid.size(); i++)
        {
            if (valid[i] == c)
            {
                std::string result = "x";
                result[0] = c;
                return (result);
            }
        }
    }
}


void CScreen::status_panel_show()
{
    show_panel(g_status_bar);
    g_status_bar_data.hidden = false;
}

void CScreen::status_panel_hide()
{
    hide_panel(g_status_bar);
    g_status_bar_data.hidden = true;
}

void CScreen::status_panel_toggle()
{
    if (status_panel_visible())
        status_panel_hide();
    else
        status_panel_show();
}

int CScreen::status_panel_height()
{
    return (g_status_bar_data.height);
}


/*
 * Set the height of the status-panel - minimum size is six.
 */
void CScreen::status_panel_height(int new_size)
{
    if (new_size >= 6)
    {
        g_status_bar_data.height = new_size;

        int x = 0;
        int y = CScreen::height() - new_size;
        int cols = CScreen::width();

        /*
         * Remove old panel/window - in the correct order.
         */
        del_panel(g_status_bar);
        delwin(g_status_bar_window);

        /*
         * Create new ones of the correct size.
         */
        g_status_bar_window = newwin(new_size, cols, y, x);
        g_status_bar = new_panel(g_status_bar_window);
        set_panel_userptr(g_status_bar, &g_status_bar_data);

        status_panel_draw();
    }
}

bool CScreen::status_panel_visible()
{
    return (!g_status_bar_data.hidden);
}

void CScreen::status_panel_title(std::string new_title)
{
    g_status_bar_data.title = new_title;
    status_panel_draw();
}

std::string CScreen::status_panel_title()
{
    return (g_status_bar_data.title);
}

std::vector < std::string > CScreen::status_panel_text()
{
    return (g_status_bar_data.text);
}

void CScreen::status_panel_append(std::string display)
{
    g_status_bar_data.text.push_back(display);
    status_panel_draw();
}

/*
 * Clear the status-panel text.
 */
void CScreen::status_panel_clear()
{
    g_status_bar_data.text.clear();
    status_panel_draw();
}

/*
 * Look up the binding for the named keystroke in our keymap(s).
 *
 * If the result is a string then execute it as a function.
 */
bool CScreen::on_keypress(const char *key)
{
    /*
     * The result of the lookup.
     */
    char *result = NULL;

    /*
     * Get the current global-mode.
     */
    std::string mode = "";

    CConfig *config = CConfig::instance();
    CConfigEntry *ent = config->get("global.mode");

    if ((ent != NULL) && (ent->type == CONFIG_STRING))
        mode = *ent->value.str;

    /*
     * Default mode.
     */
    if (mode.empty())
        mode = "message";

    /*
     * Lookup the keypress in the current-mode-keymap.
     */
    CLua *lua = CLua::instance();
    result = lua->get_nested_table("keymap", mode.c_str(), key);


    /*
     * If that failed then lookup the global keymap.
     *
     * This order ensures you can have a "global" keymap, overridden in just one mode.
     */
    if (result == NULL)
        result = lua->get_nested_table("keymap", "global", key);

    /*
     * If one/other of these lookups resulted in success then we're golden.
     */
    if (result != NULL)
        lua->execute(result);

    /*
     * We succeeded if the result wasn't NULL.
     */
    return (result != NULL);
}


int CScreen::get_colour(std::string name)
{
    std::transform(name.begin(), name.end(), name.begin(), ::tolower);

    /*
     * If the name is "unread" then return the default configured
     * colour.
     */
    if (name == "unread")
    {
        CConfig *config = CConfig::instance();
        name = config->get_string("colour.unread");

        if (name.empty())
            name = "red";
    }

    return (m_colours[name]);
}



/*
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

    /*
     * Take off the panel, if visible.
     */
    if (screen->status_panel_visible())
        height -= screen->status_panel_height();

    /*
     * Account for the fact we start from row one not zero.
     */
    height += 1;


    /*
     * If we're in simple-mode we can just draw the lines directly
     * and return - we don't need to worry about the selection-handler
     * or the calculation of the scroll-point.
     */
    if (simple)
    {
        int size = lines.size();

        for (int i = 0; i < height; i++)
        {
            if ((i + selected) < size)
            {
                std::string buf = lines.at(i + selected);

                draw_single_line(i, 0, buf, stdscr);
            }
        }

        return;
    }


    /*
     * This is complex/smooth-scrolling mode.
     *
     * We'll draw a highlighted bar, and that'll move "nicely".
     */


    int middle = (height) / 2;
    int rowToHighlight = 0;
    vectorPosition topBottomOrMiddle = NONE;

    /*
     * default to TOP if our list is shorter then the screen height
     */
    if (selected < middle || max <= height)
    {
        topBottomOrMiddle = TOP;
        rowToHighlight = selected;

        /*
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
        /*
         * The current object.
         */
        int mailIndex = max;
        int size      = lines.size();


        if (topBottomOrMiddle == TOP)
        {
            /*
             * we start at the top of the list so just use row
             */
            mailIndex = row;
        }
        else if (topBottomOrMiddle == BOTTOM)
        {
            /*
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

        if ((mailIndex < max) && (mailIndex < size))
            buf = lines.at(mailIndex);

        if (buf.empty())
            continue;

        if (row == rowToHighlight)
            wattron(stdscr, A_REVERSE | A_STANDOUT);
        else
            wattroff(stdscr, A_REVERSE | A_STANDOUT);

        draw_single_line(row, 0, buf, stdscr);
    }

    /*
     * Ensure we turn off the attribute on the last line - so that
     * any blank lines are "normal".
     */
    wattroff(stdscr, A_REVERSE | A_STANDOUT);
    wattron(stdscr, COLOR_PAIR(screen->get_colour("white")));

}


/*
 * Draw a single text line, paying attention to our colour strings.
 */
void CScreen::draw_single_line(int row, int col_offset, std::string buf, WINDOW * screen)
{

    /*
     * Get the width of the screen.
     */
    int width = CScreen::width();

    /*
     * Split the string into segments, each of which might
     * have a different colour.
     */
    std::vector<COLOUR_STRING *> parts = parse_coloured_string(buf);

    /*
     * This is a horrible hack, or an inspirational piece of coding,
     * I'm not sure which.
     *
     * We have a vector of string+colour pairs.
     *
     * To cope with handling horizontal scrolling we will now construct
     * an array for each horizontal offset - this will have a column-by
     * column pair of "colour" + "character" values.
     *
     * We will use our tokenized input to populate this array, and then
     * draw it character by character.
     *
     */
    int  col_buf[1024];  // TODO - Dynamic
    char txt_buf[1024];   // TODO - Dynamic

    /*
     * We'll fill both arrays with sane defaults - white is the default
     * colour.
     */
    int size  = ARRAY_SIZE(col_buf);
    int white = COLOR_PAIR(get_colour("white"));

    /*
     * Initialize with sane defaults.
     */
    for (int i = 0; i < size; i++)
    {
        col_buf[i] = white;
        txt_buf[i] = ' ';
    }


    /*
     * Get our global horizontal offset.
     */
    CConfig *config = CConfig::instance();
    int x = config->get_integer("global.horizontal");

    /*
     * Keep track of how wide the text is, minus the formatting markers.
     */
    int w = 0;

    /*
     * We now populate the two arrays with data for each X-position:
     *
     *     The attribute for that column.
     *     The character for that column.
     */
    for (auto it = parts.begin(); it != parts.end() ; ++it)
    {
        /*
         * Get the text/colour.
         */
        COLOUR_STRING *i = (*it);
        std::string *colour = i->colour;
        std::string *text   = i->string;

        /*
         * Add on the values to the row-settings.
         */
        for (int j = 0; j < (int)text->length() ; j++)
        {
            /*
             * Bound this to make sure we don't try to walk off
             * the end of `col_buf` or `txt_buf`.
             */
            if ((w + j) < size)
            {
                col_buf[w + j] = COLOR_PAIR(get_colour(*colour));
                txt_buf[w + j] = text->at(j);
            }
        }

        w += text->length();
    }

    /*
     * We'll never draw more than this.
     *
     * TODO: This means you can't scroll more than 1023 characters
     *       which should be fixed.
     */
    if (w > size)
        w = size;

    /*
     * Now we draw the text, character by character.
     */
    int col = 0;

    /*
     * Start from the horizontal-offset, and draw each character in
     * the appropriate colour.
     *
     * Keep track of how many columns we drew, so that we can add padding
     * if our line is too short.
     */
    for (int i = x;  i < w; i++)
    {
        wattron(screen, col_buf[i]);
        mvwprintw(screen, row, col + col_offset, "%c", txt_buf[i]);
        col += 1;
    }

    /*
     * Pad away ..
     */
    while (w < (width - col_offset))
    {
        wprintw(screen, " ");
        w += 1;
    }

    /*
     * Reset to our default colour.
     */
    wattron(screen, COLOR_PAIR(get_colour("white")));


    /*
     * Finally free the tokenized string-bits.
     */
    for (auto it = parts.begin(); it != parts.end() ; ++it)
    {
        COLOUR_STRING *i = (*it);
        delete(i->string);
        delete(i->colour);
        delete(i);
    }

}


/*
 * Parse a string into an array of "string + colour" pairs,
 * which will be useful for drawing strings.
 */
std::vector<COLOUR_STRING *> CScreen::parse_coloured_string(std::string input)
{
    std::vector<COLOUR_STRING *> results;


    /*
     * I know this is horrid.
     *
     * NOTE: We're trying to be greedy but searching from the
     * back of the string forward.  This is definitely the simpler
     * of the approaches I trialled.
     */
    pcrecpp::RE re("^(.*)\\$\\[([a-zA-Z]+)\\](.*)$");

    std::string pre;
    std::string col;
    std::string txt;

    while (re.FullMatch(input, &pre, &col, &txt))
    {
        /*
         * Allocate a structure to hold this match.
         */
        COLOUR_STRING *tmp = (COLOUR_STRING *)malloc(sizeof(COLOUR_STRING));

        /*
         * Save our match away.
         */
        tmp->colour = new std::string(col);
        tmp->string = new std::string(txt);
        results.push_back(tmp);

        input = pre;
    }

    /*
     * If input is non-empty then we have leading match.  Handle that
     * as a special case.
     */
    if (! input.empty())
    {
        /*
         * Allocate a structure to hold this match.
         */
        COLOUR_STRING *tmp = (COLOUR_STRING *)malloc(sizeof(COLOUR_STRING));
        tmp->colour = new std::string("default");
        tmp->string = new std::string(input);
        results.push_back(tmp);
    }

    /*
     * Remember we searched backwards?  Reverse so all makes sense.
     */
    std::reverse(results.begin(), results.end());
    return (results);
}
