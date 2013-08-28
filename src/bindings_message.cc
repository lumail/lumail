/**
 * bindings_message.cc - Bindings for all message-related Lua primitives.
 *
 * This file is part of lumail: http://lumail.org/
 *
 * Copyright (c) 2013 by Steve Kemp.  All rights reserved.
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
    for (std::string header : headers)
    {
        unused=write(fd, header.c_str(), header.size() );
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
     * Get the sendmail binary to use.
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
     * Call the on_send_message hook, with the path to the message.
     */
    call_message_hook( "on_send_message", filename.c_str() );


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
 * Return "true" if so.  Return false if the user said no, and either way
 * allow attachments to be added.
 */
bool should_send( lua_State * L, std::vector<std::string> *attachments )
{
    while( true )
    {
        /**
         * Use prompt_chars() to get the input
         */
        lua_pushstring(L,"Send mail: (y)es, (n)o, or (a)dd an attachment?" );
        lua_pushstring(L,"anyANY");

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
            return true;
        }
        if ( ( response[0] == 'n' ) ||
             ( response[0] == 'N' ) )
        {
            return false;
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
                return false;
            }

            const char * path = lua_tostring(L, -1);

            if ( path != NULL )
                if ( CFile::exists( path ) )
                    attachments->push_back( path );
        }
    }
}


/**
 ** Implementation of the primitives.
 **
 **/


/**
 * Get the body of the message, as displayed.
 */
int body(lua_State * L)
{
    /**
     * Get the path (optional) to the message.
     */
    const char *str = lua_tostring(L, -1);

    CMessage *msg = get_message_for_operation( str );
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

    if ( str != NULL )
        delete( msg );

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
    CMessage *mssg = get_message_for_operation( NULL );
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

    /**
     * Build up the email.
     */
    std::string filename = populate_email_on_disk(  headers, "",  sig );

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
    bool send = should_send(L, &attachments );
    if ( ! send )
    {
        call_message_hook( "on_message_aborted", filename.c_str() );
        CFile::delete_file( filename );
        return 0;
    }

    /**
     * Add attachments.  If there are none we just make the message MIME-nice.
     */
    CMessage::add_attachments_to_mail( filename, attachments );

    /**
     * Send the mail.
     */
    send_mail_and_archive( filename );

    return 0;
}


/**
 * Count messages in the selected folder(s).
 */
int count_messages(lua_State * L)
{
    CGlobal *global = CGlobal::Instance();
    std::vector<CMessage *> *messages = global->get_messages();
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
    CMessage *msg = get_message_for_operation( NULL );
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
    CMessage *msg = get_message_for_operation( NULL );
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
    const char *str = lua_tostring(L, -1);

    CMessage *msg = get_message_for_operation( str );
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
     * Free the message.
     */
    if ( str != NULL )
        delete( msg );

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
    CMessage *mssg = get_message_for_operation( NULL );
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
     * Edit the message on-disk.
     */
    CFile::edit( filename );


    /**
     * Reset the screen.
     */
    reset_prog_mode();
    refresh();


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
    bool send = should_send(L, &attachments );
    if ( ! send )
    {
        call_message_hook( "on_message_aborted", filename.c_str() );
        CFile::delete_file( filename );
        return 0;
    }

    /**
     * Add attachments.  If there are none we just make the message MIME-nice.
     */
    CMessage::add_attachments_to_mail( filename, attachments );

    /**
     * Send the mail.
     */
    send_mail_and_archive( filename );

    /**
     * Reset + redraw
     */
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
    const char *header = lua_tostring(L, 1);
    const char *path   = lua_tostring(L, 2);
    if ( header == NULL )
        return luaL_error(L, "Missing header" );

    /**
     * Get the message
     */
    CMessage *msg = get_message_for_operation( path );
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


    if ( path != NULL )
        delete( msg );

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
    const char *str = lua_tostring(L, -1);
    int ret = 0;

    CMessage *msg = get_message_for_operation( str );
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

    if ( str != NULL )
        delete( msg );

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
    const char *str = lua_tostring(L, -1);

    CMessage *msg = get_message_for_operation( str );
    if ( msg == NULL )
    {
        CLua *lua = CLua::Instance();
        lua->execute( "msg(\"" MISSING_MESSAGE "\");" );
        return( 0 );
    }

    msg->mark_read();

    if ( str != NULL )
        delete( msg );

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
    const char *str = lua_tostring(L, -1);

    CMessage *msg = get_message_for_operation( str );
    if ( msg == NULL )
    {
        CLua *lua = CLua::Instance();
        lua->execute( "msg(\"" MISSING_MESSAGE "\");" );
        return( 0 );
    }

    msg->mark_unread();

    if ( str != NULL )
        delete( msg );

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
    CMessage *mssg = get_message_for_operation( NULL );
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
     * Edit the message on-disk.
     */
    CFile::edit( filename );


    /**
     * Reset the screen.
     */
    reset_prog_mode();
    refresh();


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
    bool send = should_send(L, &attachments );
    if ( ! send )
    {
        call_message_hook( "on_message_aborted", filename.c_str() );
        CFile::delete_file( filename );
        return 0;
    }

    /**
     * Add attachments.  If there are none we just make the message MIME-nice.
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


    /**
     * Reset + redraw
     */
    return( 0 );
}


/**
 * Save the current message to a new location.
 */
int save_message( lua_State *L )
{
    const char *str = lua_tostring(L, -1);

    if (str == NULL)
        return luaL_error(L, "Missing argument to save(..)");

    if ( !CFile::is_directory( str ) )
        return luaL_error(L, "The specified destination is not a Maildir" );

    /**
     * Get the message
     */
    CMessage *msg = get_message_for_operation( NULL );
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
int scroll_message_to(lua_State *L)
{
    int offset = lua_tonumber(L, -1);
    if ( offset < 0 )
        offset = 0;

    CGlobal *global = CGlobal::Instance();
    global->set_message_offset(offset);
    return (0);
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


