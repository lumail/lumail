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
#include <assert.h>
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
 * Global pointer to the message.
 */
GMimeMessage *m_message;

/**
 * Given a single email message we want to parse than and populate
 * a vector of CAttachment objects.
 *
 * This will then be ruturned.
 */
std::vector<CAttachment> handle_mail( const char *filename )
{
    std::vector<CAttachment> results;

    GMimeParser *parser;
    GMimeStream *stream;
    int m_fd = open( filename, O_RDONLY, 0);

    if ( m_fd < 0 )
    {
        std::cerr << "Failed to open file" << std::endl;
        exit(1);
    }

    stream = g_mime_stream_fs_new (m_fd);

    parser = g_mime_parser_new_with_stream (stream);
    g_object_unref (stream);

    m_message = g_mime_parser_construct_message (parser);

    if ( m_message == NULL )
    {
        std::cerr << "Failed to construct message" << std::endl;
        exit(1);
    }

    g_object_unref (parser);


    /**
     * Create an iterator
     */
    GMimePartIter *iter =  g_mime_part_iter_new ((GMimeObject *) m_message);
    assert(iter != NULL);

    /**
     * Iterate over the message.
     */
    do
    {
        std::cout << "Found part ... " << std::endl;

        GMimeObject *part  = g_mime_part_iter_get_current (iter);

        /**
         * Get the filename - only one of these will succeed.
         */
        const char *filename = g_mime_object_get_content_disposition_parameter(part, "filename");

        if ( filename != NULL )
        {
            std::cout << "Filename: " << filename << std::endl;
        }
        else
        {
            std::cout << "Filename: NULL" << std::endl;

        }


        const char *name =  g_mime_object_get_content_type_parameter(part, "name");
        if ( name != NULL )
        {
            std::cout << "Name: " << filename << std::endl;
        }
        else
        {
            std::cout << "Name: NULL" << std::endl;

        }

        /**
         * We'll set this to the filename, if one succeeded.
         */
        const char *nm = NULL;

        if ( filename != NULL )
            nm = filename;
        else
            if ( name != NULL )
                nm = name;

        /**
         * OK did we get a filename?  If so save this away.
         */
        if ( nm != NULL )
        {

            GMimeDataWrapper *content = g_mime_part_get_content_object ((GMimePart *) part);

            GMimeStream *memstream = g_mime_stream_mem_new();


            guint8 len= g_mime_data_wrapper_write_to_stream( content, memstream );
            guint8 *b = g_mime_stream_mem_get_byte_array((GMimeStreamMem *)memstream)->data;


            char *data    = NULL;
            size_t length = 0;

            /**
             * No character set found, or it is already UTF-8.
             *
             * Save the result.
             */
            if ( b != NULL )
            {
                data = (char*)malloc( len + 1 );
                memcpy( data, b, len );
                length = (size_t)len;
            }


            /**
             * Now save it away.
             */
            results.push_back( CAttachment( nm, data, length ) );

            if ( data != NULL )
                free(data);

            g_mime_stream_close(memstream);
            g_object_unref(memstream);
        }
    }

    while (g_mime_part_iter_next (iter));

    g_mime_part_iter_free (iter);

    if ( m_message != NULL )
    {
        g_object_unref( m_message );
        m_message = NULL;
    }

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

        /**
         * Iterate over each part.
         */
        std::vector<CAttachment>::iterator it;
        for (CAttachment cur : result)
        {
            std::cout << "\tNAME: " << cur.name()
                      << " size: " << cur.size()
                      << std::endl;
        }
    }
    g_mime_shutdown();

    return ( 0 );

}
