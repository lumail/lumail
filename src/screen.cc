#include <sys/ioctl.h>
#include <cursesw.h>
#include <iostream>
#include <fstream>
#include <unistd.h>
#include <panel.h>
#include <string.h>

#include "screen.h"
#include "demo_view.h"



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
WINDOW     *g_status_bar_window;
PANEL      *g_status_bar;
PANEL_DATA g_status_bar_data;




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
CScreen * CScreen::instance ()
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

    int ch;
    int running = 1;
    while ((running > 0) && (ch = getch ()))
    {
	switch (ch)
	{
	case 'q':
	    running = 0;
	    break;
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

    for (int i = 0; i < height; i++)
    {
	mvprintw (i, 0, "%i%s", i, blank.c_str ());
    }
    doupdate ();
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
    int rows = 6;
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
    g_status_bar_data.line_one = strdup ("Lumail v2 UI demo - Toggle panel via 'tab'.  Exit via 'q'.");
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
