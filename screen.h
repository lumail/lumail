/**
 * screen.h - Utility functions related to the screen size.
 */

#ifndef _screen_h_
#define _screen_h_ 1

#include <vector>
#include "maildir.h"

/**
 * Class contains only static methods relating to the screen dimensions.
 */
class CScreen {

 public:

  /**
   * Constructor.  NOP.
   */
    CScreen();

  /**
   * Destructor.  NOP.
   */
    ~CScreen();

  /**
   * Draw/Refresh the display.
   */
    void refresh_display();

  /**
   * Setup the screen.
   */
    void Init();

  /**
   * Return the width of the screen.
   */
    static int width();

  /**
   * Return the height of the screen.
   */
    static int height();

  /**
   * Clear the status-line of the screen.
   */
    static void clearStatus();

 private:
  /**
   * Per-mode drawing primitives.
   */
    void drawMaildir();
    void drawIndex();
    void drawMessage();

};

#endif				/* _screen_h_ */
