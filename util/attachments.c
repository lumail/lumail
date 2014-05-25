/**
 * This is a proof of concept piece of code that is designed
 * to model the reworking of Lumails attachment primitves.
 *
 * Currently there are two parts of src/message.cc that deal with
 * attachments:
 *
 *   CMessage::attachments()
 *
 *   CMessage::save_attachment()
 *
 * The former iterates over the mail and returns a vector of attachment-names,
 * the latter iterates over the mail, counting attachments, and saving the
 * single one specified by offset.
 *
 * It seems obvious that we should only have one piece of code, and instead
 * we should store a vector of attachments:
 *
 *   CAttachment( filename, data, data_len )
 *
 * Given the potential complexity, and the issue with inline attachments
 * highlighted by #186 we should be careful and thus this code is born.
 *
 */


#include <string>
#include <string.h>
#include <fcntl.h>
#include <iostream>
#include <fstream>
#include <stdint.h>
#include <glib.h>
#include <glib/gstdio.h>
#include <gmime/gmime.h>
#include <vector>


#include "utfstring.h"


/**
 * Stub class to hold attachments.
 */
class CAttachment
{
public:

    /**
     * Constructor.
     */
    CAttachment(UTFString name, UTFString body, size_t sz ) {
        m_body = body;
        m_name = name;
        m_size = sz;
    };

    /**
     * Return the (file)name of the attachment.
     */
    UTFString name() { return m_name ; }

    /**
     * Return the body of the attachment.
     */
    UTFString body() { return m_body ; }

    /**
     * Return the size of the attachment.
     */
    size_t size() { return m_size ; }

private:
    UTFString m_name ;
    UTFString m_body ;
    size_t    m_size;
};





/**
 * Given a single email message we want to parse than and populate
 * a vector of CAttachment objects.
 *
 * This will then be ruturned.
 */
std::vector<CAttachment> handle_mail( const char *filename )
{
    std::vector<CAttachment> results;

    /**
     * Now do the parsing.
     */


    return( results );
}


/**
 * For each argument show the attachments
 */
int main( int argc, char *argv[] )
{

    g_mime_init(0);
    for( int i = 1; i <argc; i++ )
    {
        std::vector<CAttachment> result =  handle_mail( argv[i] );

        std::cout << "We received " << result.size()
                  << " attachment(s)." << std::endl;
    }
    g_mime_shutdown();

    return ( 0 );

}
