/**
 * message.cc - A class for working with a single message.
 */


#include <string>
#include "message.h"



/**
 * Constructo.
 */
CMessage::CMessage(std::string filename )
{
  m_path = filename;
}

/**
 * Destructor.
 */
CMessage::~CMessage()
{
}


/**
 * Get the path to the message.
 */
std::string CMessage::path()
{
  return( m_path );
}
