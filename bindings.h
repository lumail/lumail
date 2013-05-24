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

/* exit the program */
int exit(lua_State * L);

/* compose a new mail. */
int compose(lua_State *i);

/* execute a program */
int exec(lua_State * L);

/* get/set the global mode: maildir/index/message */
int global_mode(lua_State * L);

/* limit the display of folders. */
int maildir_limit(lua_State * L);

/* limit the display of messages. */
int index_limit(lua_State * L);

/* get/set the global maildir-prefix */
int maildir_prefix(lua_State * L);

/* get/set the index-format. */
int index_format(lua_State *L);

/* scroll up/down the maildir list. */
int scroll_maildir_down(lua_State * L);
int scroll_maildir_up(lua_State * L);
int jump_maildir_to(lua_State *L);

/* scroll up/down the message list. */
int scroll_index_down(lua_State * L);
int scroll_index_up(lua_State * L);
int jump_index_to(lua_State * L);

/* scroll to the folder matching the pattern. */
int scroll_maildir_to(lua_State * L);

/* scroll to the message matching the pattern. */
int scroll_index_to(lua_State * L);

/* get the current mailbox name. */
int current_maildir(lua_State * L);
int count_maildirs(lua_State *L);
int current_maildirs(lua_State *L);

/* get the current message path. */
int current_message(lua_State * L);

/* message helpers */
int is_new(lua_State *L);
int mark_new(lua_State *L);
int mark_read(lua_State *L);

/* delete a message */
int delete_message(lua_State *L);

/* draw a string at the foot of the screen. */
int msg(lua_State * L);

/* prompt for input */
int prompt(lua_State * L);
int prompt_yn(lua_State * L);

/* clear the screen */
int clear(lua_State * L);

/* refresh the display */
int refresh_display(lua_State * L);

/* sleep */
int sleep(lua_State *L );

/* set the default from address */
int from(lua_State * L);

/* get/set the path to sendmail */
int sendmail_path(lua_State *L);

/* get/set the sent folder path */
int sent_mail(lua_State *L);

/* get all known variables. */
int get_variables(lua_State *L);

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

#endif				/* _bindings_h */
