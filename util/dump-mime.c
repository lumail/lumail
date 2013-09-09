
#include <string>
#include <string.h>
#include <fcntl.h>
#include <iostream>
#include <fstream>
#include <stdint.h>
#include <glib.h>
#include <glib/gstdio.h>
#include <gmime/gmime.h>




/**
 * Dump the given header from our message, taking care of RFC 2047 decoding.
 */
void dump_header( GMimeMessage *message, const char *header )
{
    const char *value = g_mime_object_get_header ((GMimeObject *) message, header );

    if ( value )
    {
        char *decoded = g_mime_utils_header_decode_text ( value );
        std::cout << "Header - " << header << ": " << decoded << std::endl;
        g_free (decoded);
    }
    else
    {
        std::cout << "Failed to get header: " << header << std::endl;
    }

}


/**
 * Get the body of a GMimeObject part.
 */
std::string mime_part_to_text( GMimeObject *obj)
{

    GMimeContentType *content_type = g_mime_object_get_content_type (obj);

    std::string result;
    const char *charset;
    gint64 len;

    /**
     * Get the content, and create a new stream.
     */
    GMimeDataWrapper *c = g_mime_part_get_content_object( GMIME_PART(obj) );
    GMimeStream *memstream = g_mime_stream_mem_new();

    /**
     * Write the content to the memory-stream.
     */
    len = g_mime_data_wrapper_write_to_stream( c, memstream );
    guint8 *b = g_mime_stream_mem_get_byte_array((GMimeStreamMem *)memstream)->data;

    /**
     * If there is a content-type, and it isn't UTF-8 ...
     */
    if ( (charset =  g_mime_content_type_get_parameter(content_type, "charset")) != NULL &&  (strcasecmp(charset, "utf-8") != 0))
    {
        /**
         * We'll convert it.
         */
        iconv_t cv;

        cv = g_mime_iconv_open ("UTF-8", charset);
        char *converted = g_mime_iconv_strndup(cv, (const char *) b, len );
        if (converted != NULL)
        {
            result = (const char*)converted;
            g_free(converted);
        }
        else
        {
            if ( b != NULL )
                result = std::string((const char *)b, len);
        }
        g_mime_iconv_close(cv);
    }
    else
    {
        /**
         * No content type, or content-type is already correct.
         */
        if ( b != NULL )
            result = std::string((const char *)b, len);
    }
    g_mime_stream_close(memstream);
    g_object_unref(memstream);
    return( result );
}


/**
 * Dump the first text/plain part of a MIME-encoded message.
 *
 * If no such part is found try to dump the message-body regardless.
 */
void dump_mail( char *filename )
{
    GMimeMessage *m_message;
    GMimeParser *parser;
    GMimeStream *stream;
    std::string result;
    int fd;

    if ((fd = open( filename, O_RDONLY, 0)) == -1)
        return;

    stream = g_mime_stream_fs_new (fd);

    parser = g_mime_parser_new_with_stream (stream);
    g_object_unref (stream);

    m_message = g_mime_parser_construct_message (parser);
    g_object_unref (parser);


    std::cout << "Filename: " << filename << std::endl;

    /**
     * Dump some headers.
     */
    dump_header( m_message, "To" );
    dump_header( m_message, "From" );
    dump_header( m_message, "Subject" );


    /**
     * Create an iterator
     */
    GMimePartIter *iter =  g_mime_part_iter_new ((GMimeObject *) m_message);
    const char *content = NULL;

    /**
     * Iterate over the message.
     */
    do
    {
        GMimeObject *part  = g_mime_part_iter_get_current (iter);

        if ( ( GMIME_IS_OBJECT( part ) ) &&
             ( GMIME_IS_PART(part) ) )
        {
            /**
             * Get the content-type
             */
            GMimeContentType *content_type = g_mime_object_get_content_type (part);

            /**
             * Get the content if it has the right type.
             */
            if ( ( ( content_type == NULL ) || ( g_mime_content_type_is_type (content_type, "text", "plain")  ) ) &&
                ( content == NULL ) )
            {
                result = mime_part_to_text( part );
            }
        }
    }
    while (g_mime_part_iter_next (iter));

    g_mime_part_iter_free (iter);


    /**
     * If the result is empty then we'll just revert to reading the
     * message, and skipping the header.
     */
    if ( result.empty() )
    {
        GMimeObject *x = g_mime_message_get_body( m_message );
        result = mime_part_to_text( x );
    }

    g_object_unref(m_message);

    /**
     * Show the result.
     */
    std::cout << "Message Body:\n\n";
    std::cout << result << std::endl;
}


/**
 * For each argument, do the dumping.
 */
int main( int argc, char *argv[] )
{

    g_mime_init(0);
    for( int i = 1; i <argc; i++ )
    {
        dump_mail( argv[i] );
    }
    g_mime_shutdown();

    return ( 0 );

}
