/**
 * screen.h - Utility functions related to the screen size.
 */


#ifndef _screen_h_
#define _screen_h_ 1


/**
 * Class contains only static methods relating to the screen dimensions.
 */
class CScreen
{

 public:

  /**
   * Setup the screen.
   */
  static void Init();

  /**
   * Return the width of the screen.
   */
  static int width();

  /**
   * Return the height of the screen.
   */
  static int height();

};


#endif /* _screen_h_ */
