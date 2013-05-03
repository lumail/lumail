/**
 * maildir.h - Utility functions for working with Maildirs
 */


#ifndef _maildir_h_
#define _maildir_h_ 1

#include <vector>
#include <string>



/**
 * Class contains only static methods relating to the screen dimensions.
 */
class CMaildir {
 public:
  /**
   * Constructor.  NOP.
   */
  CMaildir( std::string path);

  /**
   * Destructor.  NOP.
   */
  ~CMaildir();

  /**
   * The number of new messages for this directory.
   */
  int newMessages();

  /**
   * The number of read messages for this directory.
   */
  int availableMessages();


  /**
   * The friendly name of the maildir.
   */
  std::string name();

 public:

  /**
   * Is the given path a Maildir?
   */
  static bool isMaildir(std::string path);

  /**
   * Is the given path a directory?
   */
  static bool isDirectory(std::string path);

  /**
   * Return a sorted list of maildirs beneath the given path.
   */
  static std::vector<std::string> getFolders( std::string path );

  /**
   * Count files in a directory.
   */
  static int countFiles( std::string path );

 private:

  /**
   * The path to the directory we represent.
   */
  std::string m_path;
};


#endif				/* _maildir_h_ */
