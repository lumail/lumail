/**
 * bindings_message.cc - Bindings for all message-related Lua primitives.
 *
 * This file is part of lumail: http://lumail.org/
 *
 * Copyright (c) 2013-2014 by Steve Kemp.  All rights reserved.
 *
 **
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; version 2 dated June, 1991, or (at your
 * option) any later version.
 *
 * On Debian GNU/Linux systems, the complete text of version 2 of the GNU
 * General Public License can be found in `/usr/share/common-licenses/GPL-2'
 *
 */


#include <algorithm>
#include <cursesw.h>
#include <fstream>
#include <iostream>
#include <pcrecpp.h>
#include <string.h>


#include "bindings.h"
#include "debug.h"
#include "file.h"
#include "global.h"
#include "lang.h"
#include "lua.h"
#include "maildir.h"
#include "message.h"
#include "utfstring.h"
#include "variables.h"



int unused __attribute__((unused));

/**
 * The possible actions after composing a message:
 *
 * 1. Edit the message (again).
 *
 * 2. Abort the sending.
 *
 * 3. Send the mail.
 *
 */
enum send_t { EDIT, ABORT, SEND, VIEW, RETRY };





/**
 **
 ** Implementations of utility methods that are helpful and are
 ** used solely in this file / implementation-unit.
 **
 **/




/**
 * Call a hook, with the given path.
 *
 * For example "on_edit_message", "on_send_message", or "on_message_aborted".
 */
void call_message_hook( const char *hook, const char *filename )
{
    CLua *lua = CLua::Instance();
    std::string cmd = std::string( hook ) + "(\"" + std::string(filename) + "\");";

    DEBUG_LOG( cmd );

    lua->execute( cmd );
}



/**
 * Generate and return a suitable message-id.
 */
std::string get_message_id(lua_State *L)
{
    char *name = (char *)"example.org";

    if ( hostname(L) == 1 )
    {
        name = (char *)lua_tostring(L,-1);
    }

    /**
     * Generate a new ID one.
     */
    char *message_id = g_mime_utils_generate_message_id(name);
    std::string result( message_id );
    g_free(message_id);

    return( "<"+ result +">" );
}


/**
 * Create an email on-disk, in a temporary file.
 */
std::string populate_email_on_disk( std::vector<std::string> headers, std::string body, std::string sig )
{
    /**
     * Get our temporary directory.
     */
    CGlobal *global  = CGlobal::Instance();
    std::string *tmp = global->get_variable( "tmp" );

    /**
     * Generate a temporary filename.
     */
    char filename[256] = { '\0' };
    snprintf( filename, sizeof(filename)-1, "%s/lumail.XXXXXX", tmp->c_str() );

    /**
     * 1. Open the temporary file.
     */
    int fd = mkstemp(filename);

    if (fd == -1)
        return "";


    /*
     * 2. write out each header.
     */
    bool date = false;

    for (std::string header : headers)
    {
        if ( header == "date" )
            date = true;

        unused=write(fd, header.c_str(), header.size() );
        unused=write(fd, "\n", 1 );
    }

    /*
     * If we didn't get a date - we won't - then add one.
     */
    if ( ! date )
    {
        time_t  now = time(0);
        struct tm  tstruct;
        char   buf[80];
        char *current_loc = NULL;

        tstruct = *localtime(&now);

        current_loc = setlocale(LC_TIME, NULL);

        if (current_loc != NULL)
        {
            current_loc = strdup(current_loc);
            setlocale(LC_TIME, "C");
        }

        /**
         * TODO: Make configurable.
         */
        strftime(buf, sizeof(buf), "Date: %a, %d %b %Y %H:%M:%S %z", &tstruct);


        if ( current_loc != NULL )
        {
            setlocale(LC_TIME, current_loc);
            free(current_loc);
        }

        unused = write(fd,buf, strlen(buf) );
        unused=write(fd, "\n", 1 );
    }


    /*
     * 3. write out body.
     */
    if ( ! body.empty() )
    {
        unused=write(fd, "\n", 1 );
        unused=write(fd, body.c_str(), body.size() );
    }

    /*
     * 4. write out sig.
     */
    if ( ! sig.empty() )
    {
        unused=write(fd, "\n", 1 );
        unused=write(fd, sig.c_str(), sig.size() );
    }

    close(fd);
    return( filename );

}


/**
 * Send the mail in the given file, and archive it.
 */
bool send_mail_and_archive( std::string filename )
{
    /**
     * Call the on_send_message hook, with the path to the message.
     */
    call_message_hook( "on_send_message", filename.c_str() );

    /**
     * Get the sendmail binary to use.
     *
     * NOTE: We have to do this after the hook has run, as it might
     * have updated/set the value.
     */
    CGlobal *global       = CGlobal::Instance();
    std::string *sendmail = global->get_variable("sendmail_path");

    if ( (sendmail == NULL ) ||
         (sendmail->empty() ) )
    {
        std::string error = "alert(\"You haven't defined a sendmail binary to use!\", 30 );" ;
        CLua *lua = CLua::Instance();
        lua->execute( error );
        return false;
    }


    /**
     * Send the mail.
     */
    CFile::file_to_pipe( filename, *sendmail );

    /**
     * Get a filename in the sent-mail folder.
     */
    std::string *sent_path = global->get_variable("sent_mail");
    if ( ( sent_path != NULL ) && ( ! sent_path->empty() ) )
    {
        std::string archive = CMaildir::message_in( *sent_path, false );
        if ( archive.empty() )
        {
            CFile::delete_file( filename );
            std::string error = "alert(\"Error finding file in sent-mail.\", 30 );" ;
            CLua *lua = CLua::Instance();
            lua->execute( error );
            return false;
        }


        /**
         * If we got a filename then copy the mail there.
         */
        assert( ! CFile::exists( archive ) );
        CFile::copy( filename, archive );
    }


    /**
     * Call the on_sent_message hook, with the path to the message.
     */
    call_message_hook( "on_sent_message", filename.c_str() );


    /**
     * Cleanup.
     */
    CFile::delete_file( filename );
    return true;
}


/**
 * Should we send the mail?
 *
 * Return one of three values here:
 *
 *  SEND  -> Send the mail.
 *  EDIT  -> Re-edit the mail.
 *  VIEW  -> View the mail.
 *  ABORT -> Abort the sending.
 *  RETRY -> Re-ask the question.
 *
 */
send_t should_send( lua_State * L, std::vector<std::string> *attachments )
{
    while( true )
    {
        /**
         * Use prompt_chars() to get the input
         */
        lua_pushstring(L,"Send mail: (y)es, (n)o, re(e)edit, (v)iew, or (a)dd an attachment?" );
        lua_pushstring(L,"eanvyEANVY");

        int ret = prompt_chars(L);
        if ( ret != 1 )
        {
            lua_pushstring(L, "Error receiving confirmation." );
            msg(L );
            return SEND;
        }

        const char * response = lua_tostring(L, -1);

        if (  ( response[0] == 'e' ) ||
              ( response[0] == 'E' ) )
        {
            return EDIT;
        }
        if (  ( response[0] == 'y' ) ||
              ( response[0] == 'Y' ) )
        {
            return SEND;
        }
        if (  ( response[0] == 'v' ) ||
              ( response[0] == 'V' ) )
        {
            return VIEW;
        }
        if ( ( response[0] == 'n' ) ||
             ( response[0] == 'N' ) )
        {
            return ABORT;
        }
        if ( ( response[0] == 'a' ) ||
             ( response[0] == 'A' ) )
        {
            /**
             * Add attachment.
             */
            lua_pushstring(L,"Path to attachment?" );
            ret = prompt( L);
            if ( ret != 1 )
            {
                lua_pushstring(L, "Error receiving attachment." );
                msg(L );
                return ABORT;
            }

            const char * path = lua_tostring(L, -1);

            if ( path != NULL )
            {
                if ( CFile::exists( path ) )
                {
                    attachments->push_back( path );
                }
                else
                {
                    /**
                     * Show a message.
                     */
                    std::string error = "alert(\"The specified attachment wasn't found\", 30 );" ;
                    CLua *lua = CLua::Instance();
                    lua->execute( error );
                    return RETRY;
                }
            }

        }
    }
}


/**
 ** Implementation of the primitives.
 **
 **/


/**
 * Get all headers from the current/specified message.
 */
int all_headers(lua_State * L)
{
    /**
     * Get the path (optional).
     */
    const char *path  = NULL;

    if (lua_isstring(L, -1))
        path = lua_tostring(L, 1);

    /**
     * Get the message
     */
    std::shared_ptr<CMessage> msg = get_message_for_operation( path );
    if ( msg == NULL )
    {
        CLua *lua = CLua::Instance();
        lua->execute( "msg(\"" MISSING_MESSAGE "\");" );
        return( 0 );
    }

    /**
     * Get the headers.
     */
    std::unordered_map<std::string, UTFString> headers = msg->headers();
    /**
     * Create the table.
     */
    lua_newtable(L);

    for ( auto it = headers.begin(); it != headers.end(); ++it )
    {
        std::string name = it->first;
        UTFString value  = it->second;

        lua_pushstring(L,name.c_str() );

        if ( ! value.empty() )
            lua_pushstring(L,value.c_str());
        else
            lua_pushstring(L, "[EMPTY]" );

        lua_settable(L,-3);
    }

    return(1);
}

/**
 * Get the body of the message, as displayed.
 */
int body(lua_State * L)
{
    /**
     * Get the path (optional) to the message.
     */
    const char *str  = NULL;

    if (lua_isstring(L, -1))
        str = lua_tostring(L, 1);

    std::shared_ptr<CMessage> msg = get_message_for_operation( str );
    if ( msg == NULL )
    {
        CLua *lua = CLua::Instance();
        lua->execute( "msg(\"" MISSING_MESSAGE "\");" );
        return( 0 );
    }

    /**
     * Get the body, via the on_get_body() call back.
     *
     * If that fails then get the body via parsing and filtering.
     */
    std::vector<UTFString> body;
    CLua *lua = CLua::Instance();
    body = lua->on_get_body();

    if ( body.empty() )
        body = msg->body();


    if ( body.empty() )
        lua_pushnil(L);
    else
    {
        /**
         * Convert the vector of arrays into a string.
         */
        UTFString res;
        for (UTFString bodyelement : body)
        {
            res += bodyelement;
            res += "\n";
        }

        lua_pushstring(L, res.c_str());
    }

    return( 1 );
}


/**
 * Resend a message to a new recipient.
 */
int bounce(lua_State * L)
{
    /**
     * Get the message we're replying to.
     */
    std::shared_ptr<CMessage> mssg = get_message_for_operation( NULL );
    if ( mssg == NULL )
    {
        CLua *lua = CLua::Instance();
        lua->execute( "msg(\"" MISSING_MESSAGE "\");" );
        return( 0 );
    }


    /**
     * Get the lua instance.
     */
    CLua *lua = CLua::Instance();

    /**
     * Prompt for the recipient
     */
    UTFString recipient = lua->get_input( "Bounce to: ");
    if ( recipient.empty() )
    {
        lua_pushstring(L, "Empty recipient, aborting." );
        return( msg(L ) );
    }

    /**
     * Prompt for confirmation.
     */
    bool cont = true;

    while( cont )
    {
        std::string prompt = "Bounce mail to <" + recipient + "> (y)es, (n)o?";

        lua_pushstring(L, prompt.c_str() );
        lua_pushstring(L,"nyNY");

        int ret = prompt_chars(L);
        if ( ret != 1 )
        {
            lua_pushstring(L, "Error receiving confirmation." );
            msg(L );
            return false;
        }

        const char * response = lua_tostring(L, -1);

        if (  ( response[0] == 'y' ) ||
              ( response[0] == 'Y' ) )
        {
            cont = false;
        }
        if ( ( response[0] == 'n' ) ||
             ( response[0] == 'N' ) )
        {
            return 0;
        }
    }


    /**
     * Bounce the message, from this path.
     */
    std::string path = mssg->path();

    /**
     * Get the command to execute.
     */
    CGlobal *global       = CGlobal::Instance();
    std::string *sendmail = global->get_variable("bounce_path");

    if ( (sendmail == NULL ) ||
         (sendmail->empty() ) )
    {
        std::string error = "alert(\"You haven't defined a 'bounce_path' binary to use!\", 30 );" ;
        lua->execute( error );
        return 0;
    }


    std::string cmd = *sendmail;
    cmd += " ";
    cmd += recipient;

    /**
     * Send it.
     */
    CFile::file_to_pipe( path, cmd );

    return 0;
}


/**
 * Compose a new mail.
 */
int compose(lua_State * L)
{
    /**
     * Get the lua instance.
     */
    CLua *lua = CLua::Instance();

    /**
     * Prompt for the recipient
     */
    UTFString recipient = lua->get_input( "To: ");
    if ( recipient.empty() )
    {
        lua_pushstring(L, "Empty recipient, aborting." );
        return( msg(L ) );
    }


    /**
     * Optional CC
     */
    UTFString cc = lua->get_input( "Cc: ");


    /**
     * Prompt for the subject.
     */
    UTFString subject = lua->get_input( "Subject: ", "No subject" );

    /**
     * Get the sender address.
     */
    CGlobal *global   = CGlobal::Instance();
    std::string *from = global->get_variable( "from" );

    /**
     * .signature handling.
     */
    UTFString sig = lua->get_signature( *from, recipient, subject );

    /**
     * Store the headers.
     */
    std::vector<std::string> headers;
    headers.push_back( "To: " + recipient );
    if ( ! cc.empty() )
        headers.push_back( "CC: " + cc );
    headers.push_back( "From: " + *from );
    headers.push_back( "Subject: " + subject );
    headers.push_back( "Message-ID: " + get_message_id(L) );

    /**
     * Build up the email.
     */
    std::string filename = populate_email_on_disk(  headers, "",  sig );

    /**
     * Loop with the actions menu.
     */
    while( true )
    {
        /**
         * Edit the message on-disk.
         */
        CFile::edit( filename );

        /**
         * Call the on_edit_message hook, with the path to the message.
         */
        call_message_hook( "on_edit_message", filename.c_str() );

        /**
         * Attachments associated with this mail.
         */
        std::vector<std::string> attachments;


        /**
         * Prompt for confirmation.
         */
    retry:
        send_t result = should_send(L, &attachments );

        if ( result == ABORT )
        {
            call_message_hook( "on_message_aborted", filename.c_str() );
            CFile::delete_file( filename );
            return 0;
        }

        if ( result == VIEW )
        {
            std::string cmd = "less " + filename;

            refresh();
            def_prog_mode();
            endwin();

            unused = system( cmd.c_str() );

            /**
             * Reset + redraw
             */
            reset_prog_mode();
            refresh();

            goto retry;
        }

        if ( result == SEND )
        {
            /**
             * Add attachments.
             * If there are none we just make the message MIME-nice.
             */
            CMessage::add_attachments_to_mail( filename, attachments );

            /**
             * Send the mail.
             */
            send_mail_and_archive( filename );
            return 0;
        }

        if ( result == RETRY )
            goto retry;

        /**
         * result == EDIT is implied here - so we re-loop.
         */
    }

    return 0;
}


/**
 * Count messages in the selected folder(s).
 */
int count_messages(lua_State * L)
{
    CGlobal *global = CGlobal::Instance();
    std::vector<std::shared_ptr<CMessage> > *messages = global->get_messages();
    assert(messages!=NULL);

    lua_pushinteger(L, messages->size() );
    return 1;
}


/**
 * Get the currently highlighted message-path.
 */
int current_message(lua_State * L)
{
    /**
     * Get the currently selected message.
     */
    std::shared_ptr<CMessage> msg = get_message_for_operation( NULL );
    if ( msg == NULL )
    {
        CLua *lua = CLua::Instance();
        lua->execute( "msg(\"" MISSING_MESSAGE "\");" );
        return( 0 );
    }

    /**
     * If that succeeded store the path.
     */
    std::string source = msg->path();
    if ( !source.empty() )
    {
        lua_pushstring(L, source.c_str());
        return(1);
    }
    else
    {
        return 0;
    }
}


/**
 * Count the lines in the current message.
 */
int count_lines(lua_State * L)
{
    /**
     * Get the currently selected message.
     */
    std::shared_ptr<CMessage> msg = get_message_for_operation( NULL );
    if ( msg == NULL )
    {
        CLua *lua = CLua::Instance();
        lua->execute( "msg(\"" MISSING_MESSAGE "\");" );
        return( 0 );
    }

    /**
     * If that succeeded get the body.
     */
    std::vector<UTFString> body = msg->body();
    lua_pushinteger(L, body.size() );
    return 1;
}


/**
 * Delete a message.
 */
int delete_message( lua_State *L )
{
    /**
     * Get the path (optional).
     */
    const char *str  = NULL;
    if (lua_isstring(L, -1))
        str = lua_tostring(L, 1);

    std::shared_ptr<CMessage> msg = get_message_for_operation( str );
    if ( msg == NULL )
    {
        CLua *lua = CLua::Instance();
        lua->execute( "msg(\"" MISSING_MESSAGE "\");" );
        return( 0 );
    }

    /**
     * Call the on_delete_message hook - before we remove the file
     * from disk.
     */
    call_message_hook( "on_delete_message", msg->path().c_str() );


    /**
     * Now delete the file.
     */
    CFile::delete_file( msg->path().c_str() );

    /**
     * Update messages
     */
    CGlobal *global = CGlobal::Instance();
    global->update_messages();
    global->set_message_offset(0);

    /**
     * We're done.
     */
    return 0;
}


/**
 * Forward an existing mail.
 */
int forward(lua_State * L)
{
    /**
     * Get the message we're forwarding.
     */
    std::shared_ptr<CMessage> mssg = get_message_for_operation( NULL );
    if ( mssg == NULL )
    {
        CLua *lua = CLua::Instance();
        lua->execute( "msg(\"" MISSING_MESSAGE "\");" );
        return( 0 );
    }


    /**
     * Prompt for the recipient
     */
    CLua *lua = CLua::Instance();
    UTFString recipient = lua->get_input( "To: ");
    if ( recipient.empty() )
    {
        lua_pushstring(L, "Empty recipient, aborting." );
        return( msg(L ) );
    }


    /**
     * Get the subject, and sender, etc.
     */
    std::string to      = mssg->header("To");
    std::string sender  = mssg->header("From");
    std::string sub     = mssg->header("Subject");
    std::string date    = mssg->header("Date");


    CGlobal *global   = CGlobal::Instance();
    std::string *from = global->get_variable( "from" );


    /**
     * The headers of the outgoing message we'll send.
     */
    std::vector<std::string> headers;
    headers.push_back( "To: " + recipient);
    headers.push_back( "From: " + *from);
    headers.push_back( "Subject: Fwd:" + sub);
    headers.push_back( "Message-ID: " + get_message_id(L) );


    /**
     * Body of the message we're forwarding.
     */
    std::vector<UTFString> body = mssg->body();
    std::string bbody;

    /*
     * We'll send a message of the form:
     *
     *  Forwarded message ..
     *
     *  To: $to
     *  From: $from
     *  Date: $date:
     *  Subject: $sub
     *
     *  $body
     *
     */
    bbody += "\nForwarded message ..\n\n";
    bbody += "To: " + to + "\n";
    bbody += "From: " + sender + "\n";
    bbody += "Date: " + date + "\n";
    bbody += "Subject: " + sub + "\n";
    bbody += "\n";

    int lines =(int)body.size();
    for( int i = 0; i < lines; i++ )
    {
        bbody += body[i] + "\n";
    }

    /**
     * Write it out.
     */
    std::string filename = populate_email_on_disk(  headers, bbody, "" );

    /**
     * Loop with the actions menu.
     */
    while( true )
    {

        /**
         * Edit the message on-disk.
         */
        CFile::edit( filename );

        /**
         * Call the on_edit_message hook, with the path to the message.
         */
        call_message_hook( "on_edit_message", filename.c_str() );

        /**
         * Attachments associated with this mail.
         */
        std::vector<std::string> attachments;


        /**
         * Prompt for confirmation.
         */
    retry:
        send_t result = should_send(L, &attachments );

        if ( result == RETRY )
            goto retry;

        if ( result == ABORT )
        {
            call_message_hook( "on_message_aborted", filename.c_str() );
            CFile::delete_file( filename );
            return 0;
        }

        if ( result == VIEW )
        {
            std::string cmd = "less " + filename;

            refresh();
            def_prog_mode();
            endwin();

            unused = system( cmd.c_str() );

            /**
             * Reset + redraw
             */
            reset_prog_mode();
            refresh();

            goto retry;
        }

        if ( result == SEND )
        {
            /**
             * Add attachments.
             * If there are none we just make the message MIME-nice.
             */
            CMessage::add_attachments_to_mail( filename, attachments );

            /**
             * Send the mail.
             */
            send_mail_and_archive( filename );

            return( 0 );
        }

        /**
         * Result == EDIT is implied here.
         */
    }

    return( 0 );
}


/**
 * Get a header from the current/specified message.
 */
int header(lua_State * L)
{
    /**
     * Get the path (optional), and the header (required)
     */
    const char *header = NULL;
    if (lua_isstring(L, -1))
        header = lua_tostring(L, 1);

    const char *path = NULL;
    if (lua_isstring(L, -2))
        path = lua_tostring(L, 2);

    if ( header == NULL )
        return luaL_error(L, "Missing header" );

    /**
     * Get the message
     */
    std::shared_ptr<CMessage> msg = get_message_for_operation( path );
    if ( msg == NULL )
    {
        CLua *lua = CLua::Instance();
        lua->execute( "msg(\"" MISSING_MESSAGE "\");" );
        return( 0 );
    }

    /**
     * Get the header.
     */
    std::string value = msg->header( header );
    lua_pushstring(L, value.c_str() );

    return( 1 );
}




/**
 * Is the named/current message new?
 */
int is_new(lua_State * L)
{
    /**
     * Get the path (optional).
     */
    const char *str = NULL;
    if (lua_isstring(L, -1))
        str = lua_tostring(L, 1);

    int ret = 0;

    std::shared_ptr<CMessage> msg = get_message_for_operation( str );
    if ( msg == NULL )
    {
        CLua *lua = CLua::Instance();
        lua->execute( "msg(\"" MISSING_MESSAGE "\");" );
        return( 0 );
    }
    else
    {
        if ( msg->is_new() )
            lua_pushboolean(L,1);
        else
            lua_pushboolean(L,0);

        ret = 1;
    }

    return( ret );
}


/**
 * Mark the message as read.
 */
int mark_read(lua_State * L)
{
    /**
     * Get the path (optional).
     */
    const char *str = NULL;
    if (lua_isstring(L, -1))
        str = lua_tostring(L, 1);

    std::shared_ptr<CMessage> msg = get_message_for_operation( str );
    if ( msg == NULL )
    {
        CLua *lua = CLua::Instance();
        lua->execute( "msg(\"" MISSING_MESSAGE "\");" );
        return( 0 );
    }

    msg->mark_read();

    return( 0 );
}


/**
 * Mark the message as new.
 */
int mark_unread(lua_State * L)
{
    /**
     * Get the path (optional).
     */
    const char *str = NULL;
    if (lua_isstring(L, -1))
        str = lua_tostring(L, 1);

    std::shared_ptr<CMessage> msg = get_message_for_operation( str );
    if ( msg == NULL )
    {
        CLua *lua = CLua::Instance();
        lua->execute( "msg(\"" MISSING_MESSAGE "\");" );
        return( 0 );
    }

    msg->mark_unread();

    return( 0 );
}


/**
 * Offset within the message we're displaying.
 */
int message_offset(lua_State * L)
{
    /**
     * How many lines we've scrolled down the message.
     */
    CGlobal *global = CGlobal::Instance();
    int offset = global->get_message_offset();
    assert(offset >= 0);

    lua_pushinteger(L, offset);
    return (1);
}


/**
 * Reply to an existing mail.
 */
int reply(lua_State * L)
{
    /**
     * Get the message we're replying to.
     */
    std::shared_ptr<CMessage> mssg = get_message_for_operation( NULL );
    if ( mssg == NULL )
    {
        CLua *lua = CLua::Instance();
        lua->execute( "msg(\"" MISSING_MESSAGE "\");" );
        return( 0 );
    }


    /**
     * Get the subject, and sender, etc.
     */
    std::string subject = mssg->header("Subject");
    std::string cc      = mssg->header("Cc");
    std::string ref     = mssg->header("Message-ID");

    /**
     * Do we reply to the sender, or a different place?
     */
    std::string to = mssg->header("Reply-To");
    if ( to.empty() )
        to = mssg->header("From");

    /**
     * Transform the subject.
     */
    lua_getglobal(L, "on_reply_transform_subject" );
    if (lua_isfunction(L, -1))
    {
        lua_pushstring(L, subject.c_str() );
        if (! lua_pcall(L, 1, 1, 0) )
        {
            subject = lua_tostring(L,-1);
        }
    }


    CGlobal *global   = CGlobal::Instance();
    std::string *from = global->get_variable( "from" );


    /**
     * .signature handling.
     */
    CLua     *lua = CLua::Instance();
    UTFString sig = lua->get_signature( *from, to, subject );


    /**
     * The headers.
     */
    std::vector<std::string> headers;
    headers.push_back( "To: " + to);
    if ( !cc.empty() )
        headers.push_back( "Cc: " + cc);
    headers.push_back( "From: " + *from);
    headers.push_back( "Subject: " + subject);
    headers.push_back( "Message-ID: " + get_message_id(L) );

    /**
     * If we have a message-id add that to the references.
     */
    if ( !ref.empty() )
    {
        /**
         * Message-ID might look like this:
         *
         * Message-ID: <"str"> ("comment")
         *
         * Remove the comment.
         */
        unsigned int start = 0;
        if ( ( ref.find('(') ) != std::string::npos )
        {
            size_t end = ref.find(')',start);
            if ( end != std::string::npos )
                ref.erase(start,end-start+1);
        }

        /**
         * If still non-empty ..
         */
        if ( !ref.empty() )
        {
            headers.push_back( "References: " + ref );
            headers.push_back( "In-Reply-To: " + ref );
        }
    }

    /**
     * Body
     */
    std::vector<UTFString> body = mssg->body();
    std::string bbody ;

    int lines =(int)body.size();
    for( int i = 0; i < lines; i++ )
    {
        bbody += "> " + body[i] + "\n";
    }

    /**
     * Write it out.
     */
    std::string filename = populate_email_on_disk(  headers, bbody, sig );

    /**
     * Loop with the actions menu.
     */
    while( true )
    {

        int unused __attribute__((unused));

        /**
         * Edit the message on-disk.
         */
        CFile::edit( filename );

        /**
         * Call the on_edit_message hook, with the path to the message.
         */
        call_message_hook( "on_edit_message", filename.c_str() );

        /**
         * Attachments associated with this mail.
         */
        std::vector<std::string> attachments;

        send_t result = should_send(L, &attachments );
        if ( result == ABORT )
        {
            call_message_hook( "on_message_aborted", filename.c_str() );
            CFile::delete_file( filename );
            return 0;
        }

        if ( result == VIEW )
        {
            std::string cmd = "less " + filename;

            refresh();
            def_prog_mode();
            endwin();

            unused = system( cmd.c_str() );

            /**
             * Reset + redraw
             */
            reset_prog_mode();
            refresh();

            goto retry;
        }

        if ( result == SEND )
        {
            /**
             * Add attachments.
             * If there are none we just make the message MIME-nice.
             */
            CMessage::add_attachments_to_mail( filename, attachments );

            /**
             * Send the mail.
             */
            send_mail_and_archive( filename );

            /**
             * Now we're all cleaned up mark the original message
             * as being replied to.
             */
            mssg->add_flag( 'R' );

            return 0;
        }

    retry:

        /**
         * result == EDIT is implied here.
         */
        void(0);
    }

    /**
     * All done.
     */
    return( 0 );
}


/**
 * Save the current message to a new location.
 */
int save_message( lua_State *L )
{
    const char *str = NULL;
    if (lua_isstring(L, -1))
        str = lua_tostring(L, 1);

    if (str == NULL)
        return luaL_error(L, "Missing argument to save(..)");

    if ( !CFile::is_directory( str ) )
        return luaL_error(L, "The specified destination is not a Maildir" );

    /**
     * Get the message
     */
    std::shared_ptr<CMessage> msg = get_message_for_operation( NULL );
    if ( msg == NULL )
    {
        CLua *lua = CLua::Instance();
        lua->execute( "msg(\"" MISSING_MESSAGE "\");" );
        return( 0 );
    }

    /**
     * Got a message ?
     */
    std::string source = msg->path();

    /**
     * The new path.
     */
    std::string dest = CMaildir::message_in( str, ( msg->is_new() ) );

    /**
     * Copy from source to destination.
     */
    CFile::copy( source, dest );

    /**
     * Remove source.
     */
    CFile::delete_file( source.c_str() );

    /**
     * Update messages
     */
    CGlobal *global = CGlobal::Instance();
    global->update_messages();
    global->set_message_offset(0);

    /**
     * We're done.
     */
    return 0;
}


/**
 * Scroll the message down.
 */
int scroll_message_down(lua_State *L)
{
    int step = lua_tonumber(L, -1);

    CGlobal *global = CGlobal::Instance();
    int cur = global->get_message_offset();
    cur += step;

    global->set_message_offset(cur);
    return (0);
}


/**
 * Scroll the message to the given offset.
 */
int jump_message_to(lua_State *L)
{
    int offset = lua_tonumber(L, -1);
    if ( offset < 0 )
        offset = 0;

    CGlobal *global = CGlobal::Instance();
    global->set_message_offset(offset);
    return (0);
}

/**
 * Scroll the message to the next line matching the given regexp.
 */
int scroll_message_to( lua_State *L)
{
    const char *str = NULL;
    if (lua_isstring(L, -1))
        str = lua_tostring(L, 1);

    if (str == NULL)
        return luaL_error(L, "Missing argument to scroll_message_to(..)");

    CGlobal *global = CGlobal::Instance();
    std::vector<std::shared_ptr<CMessage> > *messages = global->get_messages();

    int count = messages->size();
    int selected = global->get_selected_message();

    size_t cur_offset = global->get_message_offset();

    std::vector<UTFString> body;


    std::shared_ptr<CMessage> cur = NULL;
    if (((selected) < count) && count > 0 )
        cur = messages->at(selected);

    if ( cur == NULL )
        return 0;

    /**
     * The body might come from on_get_body.
     */
    CLua *lua = CLua::Instance();
    body = lua->on_get_body();
    if ( body.empty() )
        body = cur->body();

    /**
     * OK at this point we have "body" populated with the message
     * we're going to display.
     */
    if ( body.size() < 1 )
        return 0;

    size_t offset = cur_offset;
    offset += 1;
    if ( offset >= body.size() )
        offset = 0;

    /**
     * Iterate over the text
     */
    while( offset != cur_offset )
    {
        UTFString line = "";
        line = body.at(offset);

        if (pcrecpp::RE(str, pcrecpp::RE_Options().set_caseless(true)).PartialMatch(line.c_str()) )
        {
            /**
             * We found a match.  Jump to it.
             */
            global->set_message_offset(offset);
            return 0;
        }

        /**
         * Next line.
         */
        offset += 1;
        if ( offset >= body.size() )
            offset = 0;
    }

    return 0;
}

/**
 * Scroll the message up.
 */
int scroll_message_up(lua_State *L)
{
    int step = lua_tonumber(L, -1);

    CGlobal *global = CGlobal::Instance();
    int cur = global->get_message_offset();
    cur -= step;

    if ( cur < 0 )
        cur = 0;

    global->set_message_offset(cur);
    return (0);
}


/**
 * Send an email via lua-script.
 */
int send_email(lua_State *L)
{
    /**
     * Get the values.
     */
    lua_pushstring(L, "to" );
    lua_gettable(L,-2);
    const char *to = lua_tostring(L, -1);
    lua_pop(L, 1);
    if (to == NULL)
        return luaL_error(L, "Missing recipient.");

    lua_pushstring(L, "from" );
    lua_gettable(L,-2);
    const char *from = lua_tostring(L, -1);
    lua_pop(L, 1);
    if (from == NULL)
        return luaL_error(L, "Missing sender.");

    lua_pushstring(L, "subject" );
    lua_gettable(L,-2);
    const char *subject = lua_tostring(L, -1);
    lua_pop(L, 1);
    if (subject == NULL)
        return luaL_error(L, "Missing subject.");

    lua_pushstring(L, "body" );
    lua_gettable(L,-2);
    const char *body = lua_tostring(L, -1);
    lua_pop(L, 1);
    if (body == NULL)
        return luaL_error(L, "Missing body.");


    /**
     * Optional attachments.
     */
    lua_pushstring(L,"attachments" );
    lua_gettable(L,-2);
    std::vector<std::string> filenames;
    if ( lua_istable(L, -1 ) )
    {
        lua_pushnil(L);

        while (lua_next(L, -2))
        {
            const char *path  = lua_tostring(L, -1);
            filenames.push_back( path );
            lua_pop( L , 1);
        }

        lua_pop(L, 1);
    }
    else
        lua_pop(L, 1);


    /**
     * .signature handling.
     */
    CLua     *lua = CLua::Instance();
    UTFString sig = lua->get_signature( from, to, subject );

    /**
     * Store the headers.
     */
    std::vector<std::string> headers;
    headers.push_back( "To: " + std::string(to) );
    headers.push_back( "From: " + std::string(from) );
    headers.push_back( "Subject: " + std::string(subject) );
    headers.push_back( "Message-ID: " + get_message_id(L) );

    /**
     * Build up the email.
     */
    std::string filename = populate_email_on_disk(  headers, body,  sig );

    /**
     * Add attachments.  If there are none we just make the message MIME-nice.
     */
    CMessage::add_attachments_to_mail( filename.c_str(), filenames );

    /**
     * Send the damn email.
     */
    send_mail_and_archive( filename );

    return 0;
}


