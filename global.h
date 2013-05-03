/**
 * global.h - Singleton interface to store global data
 */

#ifndef _global_h_
#define _global_h_ 1


/**
 * A singleton class to store global data:
 *
 * current_mode:  <string>
 * current_selected_folders : vector<string>
 * index_limit : "all|unread|~p<XX>"
 * maildir_limit :  "all|unread|~p<xx>"
 *
 */
class CGlobal
{

 public:

  /**
   * Get access to the singleton instance.
   */
  static CGlobal *Instance();


  /**
   * Get/Set the current mode.
   */
  void set_mode(std::string * mode);
  std::string * get_mode();

 protected:

  /**
   * Protected functions to allow our singleton implementation.
   */
  CGlobal();
  CGlobal(const CGlobal &);
  CGlobal & operator=(const CGlobal &);


 private:

  /**
   * The single instance of this class.
   */
  static CGlobal *pinstance;

  /**
   * The mode the client is in: index, maildir, or message.
   */
  std::string * m_mode;

};

#endif				/* _global_h_ */
