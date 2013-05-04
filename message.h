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

 private:

  /**
   * The file we represent.
   */
  std::string m_path;

};


#endif /* _message_h */
