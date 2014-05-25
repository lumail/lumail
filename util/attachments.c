/**
 * This is a proof of concept piece of code that is designed
 * to model the reworking of Lumails attachment primitves.
 *
 * Currently there are two parts of src/message.cc that deal with
 * attachments
 *
 *   CMessage::attachments()
 *
 *   CMessage::save_attachment(int offset)
 *
 * The former iterates over the mail and returns a vector of attachment-names,
 * the latter iterates over the mail, counting attachments, and saving the
 * single one specified by offset.
 *
 * It seems obvious that we should only have one piece of code, and instead
 * we should store a vector of attachments which are parsed on demand
 * (and cached):
 *
 *   CAttachment( filename, data, data_len )
 *
 * Given the potential complexity, and the issue with inline attachments
 * highlighted by #186 we should be careful and thus this code is born.
 *
 * Steve
 * --
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
     * Constructor
     *
     * TODO: We should probably use `memcpy` to save the body
     * of the attachment away.
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

    std::cout << std::endl;
    std::cout << "Handling input message: " << filename << std::endl;



/**
 ** Standard setup to match CMessage ..
 **/

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
 ** REAL START OF TEST CODE
 **
 **/

    GMimePartIter *iter =  g_mime_part_iter_new ((GMimeObject *) m_message);
    assert(iter != NULL);

    /**
     * Iterate over the message.
     */
    do
    {
        GMimeObject *part  = g_mime_part_iter_get_current (iter);
        if ( (! GMIME_IS_OBJECT( part ) ) ||
             ( !GMIME_IS_PART(part) ) )
            continue;

        /**
         * Name of the attachment, if we found one.
         */
        char *aname = NULL;

        /**
         * Size of the attachment, if we found one.
         */
        size_t asize = 0;

        /**
         * Attachment content, if we found one.
         */
        char *adata = NULL;


        /**
         * Get the content-disposition, so that we can determine
         * if we're dealing with an attachment, or an inline-part.
         */
        GMimeContentDisposition *disp;
        disp = g_mime_object_get_content_disposition (part);


        if (disp != NULL && !g_ascii_strcasecmp (disp->disposition, "attachment"))
        {
            /**
             * Attempt to get the filename/name.
             */
            aname = (char *)g_mime_object_get_content_disposition_parameter(part, "filename");
            if ( aname == NULL )
                aname = (char *)g_mime_object_get_content_type_parameter(part, "name");


            /**
             * Did that work?
             */
            if ( aname == NULL )
            {
                std::cout << "\tAttachment has no name." << std::endl;
            }
            else
            {
                std::cout << "\tAttachment has name : " << aname << std::endl;
            }

        }
        else
        {
            if ( disp != NULL && disp->disposition != NULL )
                std::cout << "\tInline part with name: " << disp->disposition << std::endl;
            else
                std::cout << "\tInline part."  << std::endl;


        }


        /**
         * OK we have a name, possibly, and we have a part.
         * let us see if we can get the content from it.
         */
        GMimeDataWrapper *content = g_mime_part_get_content_object(GMIME_PART(part));
        GMimeStream    *memstream = g_mime_stream_mem_new();

        guint8 len= g_mime_data_wrapper_write_to_stream( content, memstream );
        guint8 *b = g_mime_stream_mem_get_byte_array((GMimeStreamMem *)memstream)->data;


        if ( b != NULL )
        {
            adata = (char*)malloc( len + 1 );
            if ( adata == NULL )
            {
                std::cerr << "Failed to allocate memory" << std::endl;
                exit(1);
            }

            memcpy( adata, b, len );
            asize = (size_t)len;

            std::cout << "\tSize: " << asize << std::endl;
        }

        g_mime_stream_close(memstream);
        g_object_unref(memstream);

    }

    while (g_mime_part_iter_next (iter));

    g_mime_part_iter_free (iter);

/**
 ** Standard cleanup to match CMessage ..
 **/
    if ( m_message != NULL )
    {
        g_object_unref( m_message );
        m_message = NULL;
    }

    return( results );
}


/**
 * For each argument show the attachments in the specified message.
 */
int main( int argc, char *argv[] )
{
    g_mime_init(0);

    for( int i = 1; i <argc; i++ )
    {

        /**
         * Parse the given mail-file.
         */
        std::vector<CAttachment> result =  handle_mail( argv[i] );


        /**
         * Show the initial results.
         */
        std::cout << "We received " << result.size()
                  << " attachment(s)." << std::endl;

        /**
         * Iterate over each detected attachment.
         */
        std::vector<CAttachment>::iterator it;
        for (CAttachment cur : result)
        {
            /**
             * Show some details.
             */
            std::cout << "\tNAME: " << cur.name()
                      << " size: " << cur.size()
                      << std::endl;
        }
    }


    /**
     * Cleanup.
     */
    g_mime_shutdown();

    return ( 0 );

}
