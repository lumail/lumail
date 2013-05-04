#include <string>
#include "message.h"

/**
 * A single message.
 */
CMessage::CMessage(std::string filename )
{
  m_path = filename;
}

CMessage::~CMessage()
{
}

std::string CMessage::path()
{
  return( m_path );
}
