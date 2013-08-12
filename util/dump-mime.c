
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
 * Dump the first text/plain part of a MIME-encoded message.
 *
 * If no such part is found try to dump the message-body regardless.
 */
void dump_mail( char *filename )
{
    std::string result;

    GMimeMessage *m_message;
    GMimeParser *parser;
    GMimeStream *stream;
    int fd;

    if ((fd = open( filename, O_RDONLY, 0)) == -1)
        return;

    stream = g_mime_stream_fs_new (fd);

    parser = g_mime_parser_new_with_stream (stream);
    g_object_unref (stream);

    m_message = g_mime_parser_construct_message (parser);
    g_object_unref (parser);

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

                const char *charset;
                char *converted;
                gint64 len;
                GMimeDataWrapper *c = g_mime_part_get_content_object( GMIME_PART(part) );
                GMimeStream *memstream = g_mime_stream_mem_new();

                len = g_mime_data_wrapper_write_to_stream( c, memstream );
                guint8 *b = g_mime_stream_mem_get_byte_array((GMimeStreamMem *)memstream)->data;

                if ( (charset =  g_mime_content_type_get_parameter(content_type, "charset")) != NULL &&  (strcasecmp(charset, "utf-8") != 0))
                {
                    iconv_t cv;

                    cv = g_mime_iconv_open ("UTF-8", charset);
                    converted = g_mime_iconv_strndup(cv, (const char *) b, len );
                    if (converted != NULL)
                    {
                        result = (const char*)converted;
                        g_free(converted);
                    }
                    else
                    {
                        if ( b != NULL )
                            result = (const char *)b;
                    }
                    g_mime_iconv_close(cv);
                }
                else
                {
                    if ( b != NULL )
                        result = ((const char *)b );
                }
                g_mime_stream_close(memstream);
                g_object_unref(memstream);
                return;
            }
        }
    }
    while (g_mime_part_iter_next (iter));

    g_mime_part_iter_free (iter);
    g_object_unref(m_message);



    /**
     * If the result is empty then we'll just revert to reading the
     * message, and skipping the header.
     */
    if ( result.empty() )
    {
        bool in_header = true;

        std::ifstream input ( filename );
        if ( input.is_open() )
        {
            while( input.good() )
            {
                std::string line;
                getline( input, line );

                if ( in_header )
                {
                    if ( line.length() <= 0 )
                        in_header = false;
                }
                else
                {
                    result += line;
                    result += "\n";
                }

            }
            input.close();
        }
    }

    /**
     * Show the result.
     */
    std::cout << "Filename: " << filename << std::endl;
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
