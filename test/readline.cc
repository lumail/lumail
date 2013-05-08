/**
 * Test mixing readline + ncurses.
 *
 * Result == failure.
 */

#include <iostream>
#include <cstdlib>

#include <ncurses.h>
#include <readline/readline.h>
#include <readline/history.h>

int main(int argc, char *argv[])
{
  bool curses = false;

  if ( argc > 1 )
    curses = true;

  char *buf;

  std::cout << "Readline history example" << std::endl;

  /**
   * Setup ncurses.
   */
  if ( curses ) {

    initscr();

    if (!has_colors() || (start_color() != OK)) {
	endwin();
	std::cerr << "We don't have the required colour support available."
	    << std::endl;
	exit(1);
    }

  }


  /**
   * Disable TAB-completion.
   */
  rl_bind_key('\t',rl_abort);

  while((buf = readline(":"))!=NULL)
    {
      if ( (strcmp(buf,"quit")==0) ||
           (strcmp(buf,"exit")==0) )
        break;

        std::cout << "You entered:" << buf << std::endl;

        if (buf[0]!= '\0')
          add_history(buf);
    }

  free(buf);

  if ( curses )
    endwin();

    return 0;
}
