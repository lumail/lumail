#include <sys/ioctl.h>
#include <cursesw.h>
#include <iostream>
#include <fstream>
#include <unistd.h>
#include <panel.h>
#include <string.h>

#include "demo_view.h"
#include "lua.h"
#include "screen.h"



/**
 * Data-structure associated with the status-bar.
 *
 * This is used to determine if it is visible or not, as well as the
 * lines of text that are displayed.
 */
typedef struct _PANEL_DATA
{
    int hide;
    char *title;
    char *line_one;
    char *line_two;
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
CScreen::CScreen ()
{
    /**
     * Register our view-modes.
     */
    m_views["demo"] = new CDemoView ();
}


/**
 * Gain access to our singleton object.
 */
CScreen *
CScreen::instance ()
{
    static CScreen *instance = new CScreen ();
    return (instance);
}

/**
 * Destructor.  NOP.
 */
CScreen::~CScreen ()
{
    teardown ();
}

/**
 * Run our event loop.
 */
void
CScreen::run_main_loop ()
{
  /**
   * Timeout on input every half-second.
   */
    timeout (500);

    CScreen *s = CScreen::instance ();
    s->clear ();


    int ch;
    int running = 1;
    while ((running > 0) && (ch = getch ()))
    {
	CScreen *s = CScreen::instance ();
	s->clear ();

	switch (ch)
	{
	case 'q':
	    running = 0;
	    break;
	case ':':
	    {
		std::string e = get_line ();
		CLua *lua = CLua::Instance ();
		lua->execute (e);
		break;
	    }
	case '\t':
	    if (g_status_bar_data.hide == FALSE)
	    {
		hide_panel (g_status_bar);
		g_status_bar_data.hide = TRUE;
	    }
	    else
	    {
		show_panel (g_status_bar);
		g_status_bar_data.hide = FALSE;
	    }
	    break;

	}

      /**
       * Get the global mode .. TODO .. and draw it.
       */
	CViewMode *mode = m_views["demo"];
	mode->draw ();

	/**
         * Update our panel.
         */
	update_panels ();
	doupdate ();

    }

}

/**
 * Setup the curses/screen.
 */
void
CScreen::setup ()
{
    /**
     * Setup locale.
     */
    setlocale (LC_CTYPE, "");
    setlocale (LC_ALL, "");

    char e[] = "ESCDELAY=0";
    putenv (e);

    /**
     * Setup ncurses.
     */
    initscr ();

    /**
     * Make sure we have colours.
     */
    if (!has_colors () || (start_color () != OK))
    {
	endwin ();
	std::cerr << "Missing colour support" << std::endl;
	exit (1);
    }

    /* Initialize curses */
    initscr ();
    start_color ();
    raw ();
    cbreak ();
    noecho ();
    keypad (stdscr, TRUE);
    use_default_colors ();


    /* Initialize all the colors */
    init_pair (1, COLOR_WHITE, COLOR_BLACK);
    init_pair (2, COLOR_GREEN, COLOR_BLACK);
    init_pair (3, COLOR_BLUE, COLOR_BLACK);
    init_pair (4, COLOR_YELLOW, COLOR_BLACK);

    /* Create the status-bar.  Show it */
    init_status_bar ();
}


/**
 * Shutdown curses.
 */
void
CScreen::teardown ()
{
    endwin ();
}


/**
 * Clear the screen
 */
void
CScreen::clear ()
{
  /**
   * Clear the whole screen.
   */
    int width = CScreen::width ();
    int height = CScreen::height ();

    std::string blank = "";
    while ((int) blank.length () < width)
	blank += " ";

    for (int i = 0; i <= height; i++)
    {
	mvprintw (i, 0, "%s", blank.c_str ());
    }
    update_panels ();
    doupdate ();
    refresh ();
}


/**
 * Return the height of the screen.
 */
int
CScreen::height ()
{
    struct winsize w;
    ioctl (0, TIOCGWINSZ, &w);
    return (w.ws_row);
}


/**
 * Delay for the given period.
 */
void
CScreen::sleep (int seconds)
{
    ::sleep (seconds);
}

/**
 * Return the width of the screen.
 */
int
CScreen::width ()
{
    struct winsize w;
    ioctl (0, TIOCGWINSZ, &w);
    return (w.ws_col);
}



/* Create the status-bar */
void
CScreen::init_status_bar ()
{
    int show = 1;
    int x, y;

    /*
     * Get window width/height
     */
    struct winsize size;
    if (ioctl (0, TIOCGWINSZ, (char *) &size) < 0)
	printf ("TIOCGWINSZ error");

  /**
   * Size of panel
   */
    int rows = PANEL_HEIGHT;
    int cols = size.ws_col;

  /**
   * Create the window.
   */
    x = 0;
    y = size.ws_row - rows;
    g_status_bar_window = newwin (rows, cols, y, x);

  /**
   * Set the content of the status-bar
   */
    g_status_bar_data.title = strdup ("Status Panel");
    g_status_bar_data.line_one =
	strdup
	("Lumail v2 UI demo - Toggle panel via 'tab'.  Exit via 'q'.  Eval via ':'.");
    g_status_bar_data.line_two = strdup ("by Steve Kemp");

  /**
   * Refresh the panel display.
   */
    redraw_status_bar ();

    /* Attach the panel to the window. */
    g_status_bar = new_panel (g_status_bar_window);
    set_panel_userptr (g_status_bar, &g_status_bar_data);


    if (show)
    {
	show_panel (g_status_bar);
	g_status_bar_data.hide = FALSE;
    }
    else
    {
	hide_panel (g_status_bar);
	g_status_bar_data.hide = TRUE;
    }

}

/**
 * Update the text in the status-bar.
 */
void
CScreen::redraw_status_bar ()
{
    int width;
    int height;
    getmaxyx (g_status_bar_window, height, width);
    height += 1;		// nop

    box (g_status_bar_window, 0, 0);
    mvwaddch (g_status_bar_window, 2, 0, ACS_LTEE);
    mvwhline (g_status_bar_window, 2, 1, ACS_HLINE, width - 2);
    mvwaddch (g_status_bar_window, 2, width - 1, ACS_RTEE);

  /**
   * Create a blank string that will ensure shorter/updated
   * titles/lines don't get orphaned on the screen.
   *
   * The width of the screen is the max-length of the string
   * we need to construct - but note that we subtract two:
   *
   *  One for the trailign NULL.
   *  One for the border-character at the end of the line.
   */
    char *blank = (char *) malloc (width);
    for (int i = 0; i < width - 2; i++)
	blank[i] = ' ';
    blank[width - 1] = '\0';

  /**
   * Show the title, and the two lines of text we might have.
   */
    PANEL_DATA x = g_status_bar_data;
    if (x.title)
    {
	wattron (g_status_bar_window, COLOR_PAIR (2));
	mvwprintw (g_status_bar_window, 1, 1, blank);
	mvwprintw (g_status_bar_window, 1, 1, x.title);
    }
    if (x.line_one)
    {
	wattron (g_status_bar_window, COLOR_PAIR (3));
	mvwprintw (g_status_bar_window, 3, 1, blank);
	mvwprintw (g_status_bar_window, 3, 1, x.line_one);
    }
    if (x.line_two)
    {
	wattron (g_status_bar_window, COLOR_PAIR (4));
	mvwprintw (g_status_bar_window, 4, 1, blank);
	mvwprintw (g_status_bar_window, 4, 1, x.line_two);
    }

  /**
   * Avoid a leak.
   */
    free (blank);
}


/**
 * Read a line of input via the status-line.
 */
std::string CScreen::get_line ()
{
    std::string buffer;

    int
	old_curs = curs_set (1);
    int
	pos = 0;
    int
	x,
	y;
    int
	orig_x,
	orig_y;

    /**
     * Get the cursor position
     */
    getyx (stdscr, orig_y, orig_x);

    /**
     * Determine where to move the cursor to.  If the panel is visible it'll
     * be above that.
     */
    x = 0;
    y = height () - 1;
    if (g_status_bar_data.hide == FALSE)
    {
	y -= PANEL_HEIGHT;
    }

    mvaddnstr (y, x, ":", 1);

    x = 2;
    while (true)
    {
	int
	    c;
	bool isKeyCode;

	mvaddnstr (y, x, buffer.c_str (), buffer.size ());

	/**
         * Clear the line- the "-2" comes from the size of the prompt.
         */
	for (int padding = buffer.size (); padding < (width () - 2);
	     padding++)
	    printw (" ");

	/**
         * Move the cursor
         */
	move (y, x + pos);

	/**
         * Get input - paying attention to the buffer set by 'stuff()'.
         */
	isKeyCode = ((c = getch ()) == KEY_CODE_YES);

	/**
         * Ropy input-handler.
         */
	if ((isKeyCode && c == KEY_ENTER) || (c == '\n' || c == '\r'))
	{
	    break;
	}
	else if (c == 1)	/* ctrl-a : beginning of line */
	{
	    pos = 0;
	}
	else if (c == 5)	/* ctrl-e: end of line */
	{
	    pos = buffer.size ();
	}
	else if (c == 11)	/* ctrl-k: kill to end of line */
	{
	    /**
             * Kill the buffer from the current position onwards.
             */
	    buffer = buffer.substr (0, pos);
	}
	else if ((c == 2) ||	/* ctrl-b : back char */
		 (isKeyCode && (c == KEY_LEFT)))
	{
	    if (pos > 0)
		pos -= 1;
	}
	else if ((c == 6) ||	/* ctrl-f: forward char */
		 (isKeyCode && (c == KEY_RIGHT)))
	{
	    if (pos < (int) buffer.size ())
		pos += 1;
	}
	else if (isKeyCode && (c == KEY_BACKSPACE))
	{
	    if (pos > 0)
	    {
		buffer.erase (pos - 1, 1);
		pos -= 1;
	    }
	}
	else if (c == 4)	/* ctrl+d */
	{
	    /**
             * Remove the character after the point.
             */
	    if (pos < (int) buffer.size ())
	    {
		buffer.erase (pos, 1);
	    }
	}
	else if (!isKeyCode && isprint (c))
	{
	    /**
             * Insert the character into the buffer-string.
             */
	    buffer.insert (pos, 1, c);
	    pos += 1;
	}

    }

    if (old_curs != ERR)
	curs_set (old_curs);

    /**
     * Restore cursor position.  If that matters.
     */
    //    move( orig_y, orig_x );

    return (buffer);
}
