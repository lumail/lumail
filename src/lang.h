/**
 * lang.h - Text Strings for i18n.
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


#ifndef _clang_h_
#define _clang_h_ 1


/**
 * Displayed literally if there is no colour support in the terminal.
 */
#define MISSING_COLOR_SUPPORT "We don't have the required colour support available."

/**
 * Displayed above the list of selected folders, if no messages can be found.
 */
#define NO_MESSAGES_IN_FOLDERS "No messages found in:"


/**
 * Displayed literally when no folders are selected in index-mode.
 */
#define NO_MESSAGES_NO_FOLDERS "No messages were found, because no folders are selected."


/**
 * When no messages are found matching a limit.
 */
#define NO_MESSAGES_MATCHING_FILTER "No messages matching the index-limit '%s'."

/**
 * Displayed literally if we're in message-mode but there is no current
 * message.
 */
#define NO_MESSAGES "No selected message?!"


/**
 * Displayed literally if an operation is carried out on the current
 * message, but no message is currently selected.
 */
#define MISSING_MESSAGE "Finding the current message failed."


#endif /* _clang_h_ */
