/**
 * maildir.h - Utility functions for working with Maildirs
 */


#ifndef _maildir_h_
#define _maildir_h_ 1


#include <string>



/**
 * Class contains only static methods relating to the screen dimensions.
 */
class CMaildir
{

 public:

  /**
   * Is the given path a Maildir?
   */
  static bool isMaildir( std::string path );

  /**
   * Is the given path a directory?
   */
  static bool isDirectory( std::string path );

};


#endif /* _maildir_h_ */
