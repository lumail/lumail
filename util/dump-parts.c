
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
 * Output a list of the parts included in the current message.
 */
void dump_mail( char *filename )
{
    std::string result;

    GMimeMessage *m_message;
    GMimeParser *parser;
    GMimeStream *stream;
    int fd;

    std::cout << "File: " << filename << std::endl;

    if ((fd = open( filename, O_RDONLY, 0)) == -1)
    {
        std::cout << "Opening failed ..." << std::endl;
        return;
    }

    stream = g_mime_stream_fs_new (fd);

    parser = g_mime_parser_new_with_stream (stream);
    g_object_unref (stream);

    m_message = g_mime_parser_construct_message (parser);
    g_object_unref (parser);

    /**
     * Create an iterator
     */
    GMimePartIter *iter =  g_mime_part_iter_new ((GMimeObject *) m_message);
    int count = 1;

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
            gchar *type = g_mime_content_type_to_string ( content_type );
            std::cout << count << " " << type << std::endl;
        }
        count += 1;
    }
    while (g_mime_part_iter_next (iter));

    g_mime_part_iter_free (iter);
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
        dump_mail( argv[i] );
    }
    g_mime_shutdown();

    return ( 0 );

}
