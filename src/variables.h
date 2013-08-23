/**
 * variables.h - Bindings for lua-visible variables
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

#ifndef _variables_h
#define _variables_h  1

#include "lua.h"


/**
 **  Helper functions.
 **/

/**
 * Get or set the value of the given string variable.
 */
int get_set_string_variable( lua_State *L, const char * name );



/**
 * Colour & highlight getters/setters.
 */
int attachment_colour(lua_State *L);
int body_colour(lua_State *L);
int header_colour(lua_State *L);
int unread_maildir_colour(lua_State *L);
int unread_message_colour(lua_State *L);
int maildir_highlight_mode(lua_State *L);
int index_highlight_mode(lua_State *L);

/**
 * General getters/setters.
 */
int completion_chars(lua_State *L);
int display_filter(lua_State * L);
int editor(lua_State * L);
int from(lua_State * L);
int global_mode(lua_State * L);
int history_file(lua_State *L);
int index_format(lua_State * L);
int index_limit(lua_State * L);
int mail_filter(lua_State * L);
int maildir_format(lua_State *L );
int maildir_limit(lua_State * L);
int maildir_prefix(lua_State * L);
int sendmail_path(lua_State * L);
int sent_mail(lua_State * L);
int sort(lua_State * L);


/**
 * Get the users's editor, for compose() + reply() primitives.
 */
std::string get_editor();


#endif /* _variables_h */
