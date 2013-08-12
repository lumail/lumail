/**
 * bindings.h - Bindings for all functions callable from Lua.
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
 */

#ifndef _bindings_h
#define _bindings_h  1

#include "lua.h"
class CMessage;


/**
 **
 **  Helper functions.
 **
 **/


/**
 * Get a message object for the given path.
 *
 * If the path is NULL then find and return the currently selected
 * message instead.
 */
CMessage *get_message_for_operation( const char *path );


/**
 * Call a hook, passing a filename as an argument.
 */
void call_message_hook(const char *hook, const char *filename );



/**
 **
 **  Lua primitive implementations.
 **
 **/


/* exit the program */
int exit(lua_State * L);

/* exit abnormally */
int abort(lua_State * L);

/* stuff characters into the input buffer */
int stuff(lua_State * L);

/* raise an alert message */
int alert(lua_State * L);

/* compose a new mail. */
int compose(lua_State *i);

/* reply to an existing mail */
int reply(lua_State *i);

/* send a mail via script */
int send_email(lua_State *L);

/* execute a program */
int exec(lua_State * L);

/* get/set the global mode: maildir/index/message */
int global_mode(lua_State * L);

/* the format-string for the maildir-display. */
int maildir_format(lua_State * L);

/* limit the display of folders. */
int maildir_limit(lua_State * L);

/* limit the display of messages. */
int index_limit(lua_State * L);

/* get/set the global maildir-prefix */
int maildir_prefix(lua_State * L);

/* get/set the editor */
int editor(lua_State *L);

/* get/set completion characters */
int completion_chars(lua_State *L);

/* get/set the index-format. */
int index_format(lua_State *L);

/* scroll up/down the maildir list. */
int scroll_maildir_down(lua_State * L);
int scroll_maildir_up(lua_State * L);
int jump_maildir_to(lua_State *L);

/* scroll up/down the index. */
int scroll_index_down(lua_State * L);
int scroll_index_up(lua_State * L);
int jump_index_to(lua_State * L);

/* scroll up/down the message */
int scroll_message_down(lua_State * L);
int scroll_message_up(lua_State * L);
int scroll_message_to(lua_State * L);

/* scroll to the folder matching the pattern. */
int scroll_maildir_to(lua_State * L);

/* scroll to the message matching the pattern. */
int scroll_index_to(lua_State * L);

/* get the current mailbox name. */
int current_maildir(lua_State * L);
int count_maildirs(lua_State *L);
int current_maildirs(lua_State *L);
int select_maildir(lua_State *L);
int maildirs_matching(lua_State *L);

/* get the current message path. */
int current_message(lua_State * L);

/* count lines in message. */
int count_lines(lua_State *L);

/* message helpers */
int is_new(lua_State *L);
int mark_unread(lua_State *L);
int mark_read(lua_State *L);
int header(lua_State *L);

/* delete a message */
int delete_message(lua_State *L);

/* move a message */
int save_message( lua_State *L );

/* draw a string at the foot of the screen. */
int msg(lua_State * L);

/* get the mime-type of a file. */
int mime_type(lua_State *L);

/* prompt for input */
int prompt(lua_State * L);
int prompt_chars(lua_State *L);
int prompt_yn(lua_State * L);
int prompt_maildir(lua_State *L);

/* clear the screen */
int clear(lua_State * L);

/* show brief help. */
int show_help(lua_State *L);

/* refresh the display */
int refresh_display(lua_State * L);

/* sleep */
int sleep(lua_State *L );

/* get/set the default from address */
int from(lua_State * L);

/* get/set the path to sendmail */
int sendmail_path(lua_State *L);

/* get/set the sent folder path */
int sent_mail(lua_State *L);

/* get/set the message sort-order */
int sort(lua_State *L);

/* get all known variables. */
int get_variables(lua_State *L);

/**
 * Colour support.
 */
int attachment_colour(lua_State *L);
int body_colour(lua_State *L);
int header_colour(lua_State *L);
int unread_maildir_colour(lua_State *L);
int unread_message_colour(lua_State *L);

/**
 * folder selection code.
 */
int selected_folders(lua_State * L);
int clear_selected_folders(lua_State * L);
int add_selected_folder(lua_State * L);
int toggle_selected_folder(lua_State * L);
int set_selected_folder(lua_State * L);
int count_messages(lua_State * L);

/**
 * Accessors for the screen dimensions.
 */
int screen_width(lua_State * L);
int screen_height(lua_State * L);

/**
 * File/Utility handlers. Useful for writing portable configuration files.
 */
int create_maildir(lua_State *L);
int executable(lua_State *L);
int file_exists(lua_State *L);
int is_directory(lua_State *L);
int is_maildir(lua_State *L);
int load_directory(lua_State *L);

/**
 * Attachment handling.
 */
int attachments(lua_State *L);
int count_attachments(lua_State *L);
int save_attachment(lua_State *L);

/**
 * Debugging code.
 */
int lua_dump_stack(lua_State *L);
int log_message(lua_State *L);

#endif /* _bindings_h */
