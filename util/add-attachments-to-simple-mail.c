/**
 * Given a "simple message".  Add attachments to it, using GMime, and write out
 * a new file with the updated result.
 *
 * NOTE: We've discovered that GMime won't let you output to the same file as the
 * input, because of the way it looks things up on demand.
 *
 * So you must output to a *new* file, and later replace that if you wish to
 * effect an in-place expansion.
 *
 */

#include <string>
#include <string.h>
#include <fcntl.h>
#include <cstdlib>
#include <iostream>
#include <fstream>
#include <stdint.h>
#include <vector>
#include <glib.h>
#include <glib/gstdio.h>
#include <gmime/gmime.h>



int main( int argc, char *argv[] )
{
    /**
     * Create the input file.
     */
    FILE* input = fopen("input.txt", "w");
    if ( input != NULL )
    {
        const char *txt =
            "To:steve@steve.org.uk\n"
            "From: bob@example.com\n"
            "Subject: This is a test\n"
            "\n"
            "This is my body\n"
            "This is my text\n"
            "-- "
            "Steve\n";

        fwrite( txt, strlen(txt), 1, input );
        fclose( input );
    }


    /**
     * Now we're going to parse that message, and add a pair of attachments.
     */
    g_mime_init(0);


    GMimeMessage *message;
    GMimeParser  *parser;
    GMimeStream  *stream;
    int fd;



    if ((fd = open ( "input.txt", O_RDONLY, 0)) == -1)
        return -1;

    stream = g_mime_stream_fs_new (fd);

    parser = g_mime_parser_new_with_stream (stream);
    g_object_unref (stream);

    message = g_mime_parser_construct_message (parser);
    g_object_unref (parser);


    GMimeMultipart *multipart;
    GMimePart *attachment;
    GMimeDataWrapper *content;

    /**
     * Create a new multipart message.
     */
    multipart = g_mime_multipart_new();
    GMimeContentType *type = g_mime_content_type_new ("multipart", "mixed");
    g_mime_object_set_content_type (GMIME_OBJECT (multipart), type);



    GMimeContentType *new_type;
    GMimeObject *mime_part;

    mime_part = g_mime_message_get_mime_part (message);
    new_type = g_mime_content_type_new_from_string ("text/plain; charset=UTF-8");
    g_mime_object_set_content_type (mime_part, new_type);
    g_object_unref (new_type);

    /**
     * first, add the message's toplevel mime part into the multipart
     */
    g_mime_multipart_add (multipart, g_mime_message_get_mime_part (message));

    /**
     * now set the multipart as the message's top-level mime part
     */
    g_mime_message_set_mime_part (message,(GMimeObject*) multipart);

    /**
     * The files we'll attach.
     */
    std::vector<std::string> attachments;
    attachments.push_back( "/etc/passwd" );
    attachments.push_back( "/etc/fstab" );

    std::vector<std::string>::iterator it;
    for (it = attachments.begin(); it != attachments.end(); ++it)
    {
        std::string name = (*it);

        if ((fd = open (name.c_str(), O_RDONLY)) == -1)
            return -1;

        stream = g_mime_stream_fs_new (fd);

        /**
         * the stream isn't encoded, so just use DEFAULT
         */
        content = g_mime_data_wrapper_new_with_stream (stream, GMIME_CONTENT_ENCODING_DEFAULT);

        g_object_unref (stream);

        /**
         * if you knew the mime-type of the file, you could use that instead
         * of application/octet-stream
         */
        attachment = g_mime_part_new_with_type ("application", "octet-stream");
        g_mime_part_set_content_object (attachment, content);
        g_object_unref (content);

        /**
         * set the filename?
         */
        g_mime_part_set_filename (attachment, basename (name.c_str()));

        /**
         * NOTE: We might want to base64 encode this for transport...
         *
         * NOTE: if you want o get really fancy, you could use
         * g_mime_part_get_best_content_encoding()
         * to calculate the most efficient encoding algorithm to use.
         */
        g_mime_part_set_content_encoding (attachment, GMIME_CONTENT_ENCODING_BASE64);


        /**
         * Add the attachment to the multipart
         */
        g_mime_multipart_add (multipart, (GMimeObject*)attachment);
        g_object_unref (attachment);
    }


    /**
     * now that we've finished referencing the multipart directly (the message still
     * holds it's own ref) we can unref it.
     */
    g_object_unref (multipart);

    /**
     * Output the the updated message, which now contains the attachments,
     * and is well-formed.
     */
    FILE *f = NULL;
    if ((f = fopen ( "message.out","wb")) == NULL)
    {
        return -1;
    }

    GMimeStream *ostream = g_mime_stream_file_new (f);
    g_mime_object_write_to_stream ((GMimeObject *) message, ostream);
    g_object_unref(ostream);

    /**
     * Show the results.
     */
    std::cout <<  "Converted input.txt into message.out\n" << std::endl;

    return ( 0 );

}
