#include <panel.h>
#include <string.h>
#include <malloc.h>
#include <time.h>
#include <sys/ioctl.h>


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
 * Redraw the screen.
 */
void update_screen ();


/**
 * Create the status-bar.
 */
void init_status_bar (int show);

/**
 * Update the text in the status-bar.
 */
void redraw_status_bar();




int
main ()
{
    int ch;

    /* Initialize curses */
    initscr ();
    start_color ();
    cbreak ();
    noecho ();
    keypad (stdscr, TRUE);

    /* Initialize all the colors */
    init_pair (1, COLOR_WHITE, COLOR_BLACK);
    init_pair (2, COLOR_GREEN, COLOR_BLACK);
    init_pair (3, COLOR_BLUE, COLOR_BLACK);
    init_pair (4, COLOR_YELLOW, COLOR_BLACK);

    /* Create the status-bar.  Show it */
    init_status_bar (1);

    update_screen ();

  /**
   * Timeout every half-second.
   */
    timeout (500);


  /**
   * Loop.
   */
    int running = 1;
    while ((running > 0) && (ch = getch ()))
    {
	switch (ch)
	{
	case 'q':
	    running = 0;
	    break;
	case 'a':
	    g_status_bar_data.title = strdup ("Updated Status Panel");
	    redraw_status_bar();

	    break;
	case 'b':
	    g_status_bar_data.title = strdup ("I like cakes");
	    redraw_status_bar();
	    break;

	case 'c':
	    g_status_bar_data.title = strdup ("Short title.");
	    redraw_status_bar();
	    break;

	case 'd':
	    g_status_bar_data.title = strdup ("Word.");
	    g_status_bar_data.line_one = strdup ("Adverb.");
	    g_status_bar_data.line_two = strdup ("Noun.");
	    redraw_status_bar();
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

	update_screen ();
    }
    endwin ();
    return 0;
}


/**
 * This refreshes the screen.
 */
void
update_screen ()
{
    /* Show a message to the screen */
    attron (COLOR_PAIR (1));
    for (int i = 0; i < LINES; i++)
    {
	if (g_status_bar_data.hide == TRUE)
	    mvprintw (i, 0,
		      "Show the status-panel with 'TAB' - This is row %d - Time: %d",
		      i, time (NULL));
	else
	    mvprintw (i, 0,
		      "Hide the status-panel with 'TAB' - This is row %d - Time:% d",
		      i, time (NULL));

    }

    update_panels ();
    doupdate ();
}

/* Create the status-bar */
void
init_status_bar (int show)
{

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
    g_status_bar_data.line_one = strdup ("Lumail v2 UI demo - Press 'a', 'b', 'c', or 'd' to update.");
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
redraw_status_bar ()
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
