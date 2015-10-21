#ifndef _MESSAGE_PART_H
#define _MESSAGE_PART_H 1

#include <string>

/**
 * This is the C++ object which represents a MIME-part from a message.
 *
 */
class CMessagePart
{
  public:

  /**
   * Constructor.
   */
    CMessagePart (std::string type, std::string filename, void *content,
		  size_t content_length);

  /**
   * Destructor.
   */
        ~CMessagePart ();

  /**
   * Get the content-type of the MIME-part.
   */
         std::string type ();

  /**
   * Get the filename - only makes sense for "is_attachment() == true".
   */
         std::string filename ();

  /**
   * Is this an attachment?
   */
    bool is_attachment ();

  /**
   * Get the content.
   */
    void *content ();

  /**
   * Get the length of the content.
   */
    size_t content_size ();

  private:

  /**
   * The content-type
   */
           std::string m_type;

  /**
   * The filename - if this is an attachment.
   */
           std::string m_filename;

  /**
   * The content of this MIME-part
   */
    void *m_content;

  /**
   * The content-length.
   */
    size_t m_content_length;
};



#endif /* _MESSAGE_PART_H  */
