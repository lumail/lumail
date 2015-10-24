#ifndef _SCREEN_H
#define _SCREEN_H 1

#include <string>
#include <unordered_map>


/**
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
    virtual void draw () = 0;
};


/**
 * This class contains simple functions relating to the screen-handling.
 */
class CScreen
{

  private:
    CScreen ();

  /**
   * Destructor.  NOP.
   */
    ~CScreen ();

  public:

  /**
   * Instance accessor - this is a singleton.
   */
    static CScreen *instance ();


  /**
   * Setup/Teardown
   */
    void setup ();
    void teardown ();

    /**
     * Run our event loop.
     */
    void run_main_loop ();

  /**
   * Return the width of the screen.
   */
    int width ();

  /**
   * Return the height of the screen.
   */
    int height ();

  /**
   * Clear the screen.
   */
    void clear ();

  /**
   * Delay for the given period.
   */
    void sleep (int seconds);

    /**
     * Read a line of input via the status-line.
     */
        std::string get_line ();

  private:
    void redraw_status_bar ();
    void init_status_bar ();

  private:

    /**
     * This contains the mapping of "global.mode" -> drawing routines.
     */
         std::unordered_map < std::string, CViewMode * >m_views;
};


#endif
