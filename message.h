#ifndef _message_h
#define _message_h 1

#include <string>


/**
 * A single message.
 */
class CMessage
{
 public:
  CMessage(std::string filename);
  ~CMessage();

  std::string path();

 private:
  std::string m_path;

};


#endif /* _message_h */
