
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
 * Dump all headers.
 */
void dump_headers( char *filename )
{
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


    std::cout << "Filename: " << filename << std::endl;

    const char *name;
    const char *value;

    /**
     * Prepare to iterate.
     */
    GMimeHeaderList *ls   = GMIME_OBJECT (m_message)->headers;
    GMimeHeaderIter *iter = g_mime_header_iter_new ();

    if (g_mime_header_list_get_iter (ls, iter) && g_mime_header_iter_first (iter))
    {
        while (g_mime_header_iter_is_valid (iter))
        {
            /**
             * Get the name + value.
             */
            name = g_mime_header_iter_get_name (iter);
            value = g_mime_header_iter_get_value (iter);

            /**
             * Decode the value.
             */
            char *decoded = g_mime_utils_header_decode_text ( value );

            /**
             * Store the result.
             */
            std::cout << name << ": " << decoded << std::endl;

            if (!g_mime_header_iter_next (iter))
                break;
        }
    }
    g_mime_header_iter_free (iter);


    g_object_unref(m_message);
}


/**
 * For each argument, do the dumping.
 */
int main( int argc, char *argv[] )
{

    g_mime_init(0);
    for( int i = 1; i <argc; i++ )
    {
        dump_headers( argv[i] );
    }
    g_mime_shutdown();

    return ( 0 );

}
