
#include <string>
#include <fcntl.h>
#include <iostream>
#include <fstream>
#include <stdint.h>
#include <glib.h>
#include <glib/gstdio.h>
#include <gmime/gmime.h>



int main( int argc, char *argv[] )
{

    std::string result;
    g_mime_init(0);

    GMimeMessage *m_message;
    GMimeParser *parser;
    GMimeStream *stream;
    int fd;

    if ((fd = open( argv[1], O_RDONLY, 0)) == -1)
        return 0;

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
                GMimeDataWrapper *c = g_mime_part_get_content_object( GMIME_PART(part) );
                GMimeStream *stream = g_mime_stream_mem_new();

                g_mime_data_wrapper_write_to_stream( c, stream );
                GByteArray *b = ((GMimeStreamMem *)stream)->buffer;
                result = ((const char *)b->data );
                result = result.substr(0, b->len );
                g_object_unref(stream);
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
        bool in_header = true;

        std::ifstream input ( argv[1] );
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


    std::cout << result << std::endl;
    return ( 0 );

}
