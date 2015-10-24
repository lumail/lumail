
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

};
