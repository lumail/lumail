
#include <string>
#include <vector>

#include "message_part.h"



/**
 * Constructor.
 */
CMessagePart::CMessagePart( std::string type, std::string filename, void *content, size_t content_length )
{
  m_type       = type;
  m_filename   = filename;

  m_content        = NULL;
  m_content_length = 0;

  if ( ( content_length >0 ) && ( content != NULL ) ) {
    m_content = content;
    m_content_length = content_length;
  }
}

/**
 * Destructor.
 */
CMessagePart::~CMessagePart()
{
  if ( m_content != NULL )
    {
      free(m_content);
      m_content        = NULL;
      m_content_length = 0;
    }
    
}

/**
 * Get the content-type of the MIME-part.
 */
std::string CMessagePart::type()
{
  return( m_type );
}

/**
 * Get the filename - only makes sense for "is_attachment() == true".
 */
std::string CMessagePart::filename()
{
  return( m_filename );
}


/**
 * Is this an attachment?
 */
bool CMessagePart::is_attachment()
{
  return( m_filename.empty() == false );
}
  
    
/**
 * Get the content.
 */
void *CMessagePart::content()
{
  return( m_content );
}

/**
 * Get the length of the content.
 */
size_t CMessagePart::content_size()
{
  return( m_content_length );
}

  