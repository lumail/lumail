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

/**
 * Get the flags for this message.
 */
std::string CMessage::flags()
{
  std::string flags;

  if ( m_path.empty() )
    return( flags );

  unsigned offset = m_path.find( ":2," );

  if ( offset != std::string::npos )
    flags = m_path.substr(offset+3);

  return flags;
}

