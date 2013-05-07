/**
 * message.h - A class for working with a single message.
 */

#ifndef _message_h
#define _message_h 1

#include <string>

/**
 * A single message.
 */
class CMessage
{
 public:

  /**
   * Constructor
   */
  CMessage(std::string filename);

  /**
   * Destructor.
   */
  ~CMessage();

  /**
   * Get the path.
   */
  std::string path();


  /**
   * Format the message for display in the header - via the lua format string.
   */
  std::string format();

  /**
   * Get the flags for this message.
   */
  std::string flags();

 private:

  /**
   * The file we represent.
   */
  std::string m_path;

};

#endif				/* _message_h */
