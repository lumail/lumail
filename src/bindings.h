/**
 * bindings.h - Bindings for all functions callable from Lua.
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
 */

#ifndef _bindings_h
#define _bindings_h  1

#include "lua.h"


/**
 * Forward declaration of class.
 */
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
 **
 **  Lua primitive implementations.
 **
 **/


/**
 * bindings_file.cc:
 */
int create_maildir(lua_State *L);
int cd(lua_State *L);
int cwd(lua_State *L);
int delete_maildir(lua_State *L);
int executable(lua_State *L);
int file_exists(lua_State *L);
int is_directory(lua_State *L);
int is_maildir(lua_State *L);
int load_directory(lua_State *L);


/**
 * bindings_folders.cc:
 */
int selected_folders(lua_State * L);
int clear_selected_folders(lua_State * L);
int add_selected_folder(lua_State * L);
int toggle_selected_folder(lua_State * L);
int set_selected_folder(lua_State * L);

/**
 * bindings_index.cc:
 */
int index_offset(lua_State *L);
int jump_index_to(lua_State * L);
int scroll_index_down(lua_State * L);
int scroll_index_to(lua_State * L);
int scroll_index_up(lua_State * L);

/**
 * bindings_message.cc:
 */
int all_headers(lua_State * L);
int body(lua_State * L);
int bounce(lua_State * L);
int compose(lua_State * L);
int delete_message( lua_State *L );
int forward(lua_State * L);
int header(lua_State * L);
int is_new(lua_State * L);
int mark_read(lua_State * L);
int mark_unread(lua_State * L);
int reply(lua_State * L);
int save_message( lua_State *L );
int scroll_message_down(lua_State *L);
int jump_message_to(lua_State *L);
int scroll_message_to(lua_State *L);
int scroll_message_up(lua_State *L);
int send_email(lua_State *L);


/**
 * bindings_mime.cc:
 */
int attachment(lua_State *L);
int attachments(lua_State *L);
int count_attachments(lua_State *L);
int count_body_parts(lua_State *L);
int get_body_part(lua_State *L);
int get_body_parts(lua_State *L);
int has_body_part(lua_State *L);
int save_attachment(lua_State *L);


/**
 * bindings_prompts.cc:
 */
int choose_string(lua_State * L);
int prompt(lua_State * L);
int prompt_yn(lua_State * L);
int prompt_chars(lua_State *L);
int prompt_maildir(lua_State * L);


/**
 * bindings_text.cc:
 */
int scroll_text_down(lua_State *L);
int scroll_text_to(lua_State *L);
int scroll_text_up(lua_State *L);
int show_file_contents(lua_State *L);
int show_text(lua_State *L);

/**
 * bindings.cc
 */
int abort(lua_State * L);
int alert(lua_State * L);
int bind_socket(lua_State * L);
int clear(lua_State * L);
int close_socket(lua_State * L);
int count_lines(lua_State * L);
int count_maildirs(lua_State *L);
int count_messages(lua_State * L);
int current_maildir(lua_State * L);
int current_maildirs(lua_State *L);
int current_message(lua_State * L);
int exec(lua_State * L);
int exit(lua_State * L);
int get_variables(lua_State *L );
int hostname(lua_State *L );
int jump_maildir_to(lua_State * L);
int log_message(lua_State *L);
int lua_dump_stack(lua_State *L);
int maildir_offset(lua_State *L);
int maildirs_matching(lua_State *L);
int message_offset(lua_State * L);
int mime_type(lua_State *L);
int msg(lua_State * L);
int screen_height(lua_State * L);
int screen_width(lua_State * L);
int scroll_maildir_down(lua_State * L);
int scroll_maildir_to(lua_State * L);
int scroll_maildir_up(lua_State * L);
int select_maildir(lua_State *L);
int show_help(lua_State * L);
int sleep(lua_State *L );
int stuff(lua_State * L);



#endif /* _bindings_h */
