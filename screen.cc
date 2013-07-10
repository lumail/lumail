/**
 * screen.cc - Utility functions related to the screen.
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

#include <algorithm>
#include <cstdlib>
#include <iostream>
#include <algorithm>
#include <fstream>
#include <string.h>
#include <cctype>
#include <sys/ioctl.h>
#include <dirent.h>
#include <ncurses.h>

#include "lang.h"
#include "lua.h"
#include "global.h"
#include "history.h"
#include "message.h"
#include "screen.h"

/**
 * Constructor.  NOP.
 */
CScreen::CScreen()
{
}

/**
 * Destructor.  NOP.
 */
CScreen::~CScreen()
{
}

/**
 * This function will draw the appropriate screen, depending upon our current mode.
 */
void CScreen::refresh_display()
{
    /**
     * Clear the main-part of the screen.
     */
    CScreen::clear_main();

    /**
     * Get the current mode.
     */
    CGlobal *global = CGlobal::Instance();
    std::string * s = global->get_variable("global_mode");

    if (strcmp(s->c_str(), "maildir") == 0)
	drawMaildir();
    else if (strcmp(s->c_str(), "index") == 0)
	drawIndex();
    else if (strcmp(s->c_str(), "message") == 0)
	drawMessage();
    else {
        move(3, 3);
        printw("UNKNOWN MODE: '%s'", s->c_str());
    }
}

/**
 * Draw a list of folders.
 */
void CScreen::drawMaildir()
{
    /**
     * Get all known folders + the current display mode
     */
    CGlobal *global = CGlobal::Instance();
    std::vector < CMaildir > display = global->get_folders();
    std::string *limit = global->get_variable("maildir_limit");

    /**
     * The number of items we've found, vs. the size of the screen.
     */
    int count = display.size();
    int height = CScreen::height();
    int selected = global->get_selected_folder();

    /**
     * If we have no messages report that.
     */
    if ( count < 1 )
    {
        move(2, 2);
        printw("No maildirs found matching the limit '%s'.", limit->c_str());
        return;
    }


    /**
     * Bound the selection.
     */
    if (selected >= count) {
	global->set_selected_folder(0);
	selected = 0;
    }

    int row = 0;

    /**
     * Selected folders.
     */
    std::vector < std::string > sfolders = global->get_selected_folders();

    for (row = 0; row < (height - 1); row++) {
        int unread = 0;

	move(row, 0);
	printw("  " );

        /**
         * The current object.
         */
	CMaildir *cur = NULL;
	if ((row + selected) < count) {
	    cur = &display[row + selected];
            unread = cur->newMessages();
        }

        /**
         * Is this folder part of our selected set?
         */
        bool selected = false;
	if (cur != NULL) {
	    if (std::find(sfolders.begin(), sfolders.end(), cur->path()) != sfolders.end())
		selected = true;
	}

        /**
         * First row is the current one.
         */
	if (row == 0)
          attron(A_STANDOUT);


        /**
         * The item we'll draw for this row.
         */
        std::string display = "";

        /**
         * Format.
         */
        if ( cur != NULL )
            display = cur->format( selected );

        /**
         * Overwrite the full length.
         */
	while ((int)display.size() < (CScreen::width() - 3))
            display += std::string(" ");

	move(row, 2);

        if ( unread )
        {
            if ( row == 0 )
                attrset( COLOR_PAIR(1) |A_REVERSE );
            else
                attrset( COLOR_PAIR(1) );
        }
	printw("%s", display.c_str());

        attrset( COLOR_PAIR(2) );

        /**
         * Remove the inverse.
         */
	if (row == 0)
	    attroff(A_STANDOUT);
    }
}

/**
 * Draw the index-mode.
 */
void CScreen::drawIndex()
{
    /**
     * Get all messages from the currently selected maildirs.
     */
    CGlobal *global = CGlobal::Instance();
    std::vector<CMessage*> *messages = global->get_messages();

    /**
     * If we have no messages report that.
     */
    if (( messages == NULL ) ||  (messages->size() < 1))
    {
        std::vector<std::string> folders = global->get_selected_folders();

        if ( folders.size() < 1 )
        {
            /**
             * No folders selected, and no messages.
             */
            move(2,2);
            printw( NO_MESSAGES_NO_FOLDERS );
            return;
        }

        /**
         * Show the selected folders.
         */
        move(2, 2);
        printw( NO_MESSAGES_IN_FOLDERS );

        std::vector<std::string>::iterator it;
        int height = CScreen::height();
        int row = 4;

        for (it = folders.begin(); it != folders.end(); ++it) {

            /**
             * Avoid drawing into the status area.
             */
            if ( row >= (height-1) )
                break;

            /**
             * Show the name of the folder.
             */
            std::string name = (*it);
            move( row, 5 );
            printw("%s", name.c_str() );
            row+=1;
        }
        return;
    }


    /**
     * The number of items we've found, vs. the size of the screen.
     */
    int count = messages->size();
    int height = CScreen::height();
    int selected = global->get_selected_message();

    /*
     * Bound the selection.
     */
    if (selected >= count) {
        selected = count-1;
        global->set_selected_message(selected);
    }

    /**
     * OK so we have (at least one) selected maildir and we have messages.
     */
    int row = 0;

    for (row = 0; row < (height - 1); row++)
    {
	move(row, 0);
	printw("  " );

        /**
         * What we'll output for this row.
         */
	std::string  buf;

        /**
         * The current object.
         */
	CMessage *cur = NULL;
	if ((row + selected) < count)
            cur = messages->at(row + selected);

        bool unread = false;
        if ( cur != NULL ) {
            std::string flags = cur->flags();
            if ( flags.find( "N" ) != std::string::npos )
                unread = true;
        }

	if ( unread ) {
            if (row == 0)
                attrset(COLOR_PAIR(1)|A_REVERSE);
            else
                attrset(COLOR_PAIR(1));
        }
        else {
            if (row == 0)
                attrset(A_REVERSE);
        }

	std::string path = "";

	if (cur != NULL)
            buf =  cur->format();

        /**
         * Pad.
         */
	while ((int)buf.size() < (CScreen::width() - 3))
            buf += std::string(" ");
        /**
         * Truncate.
         */
	if ((int)buf.size() > (CScreen::width() - 3))
	    buf[(CScreen::width() - 3)] = '\0';

	move(row, 2);
	printw("%s", buf.c_str());

        attrset( COLOR_PAIR(2) );

        /**
         * Remove the inverse.
         */
	if (row == 0)
	    attroff(A_REVERSE);
    }
}



/**
 * Draw the message mode.
 */
void CScreen::drawMessage()
{
    /**
     * Get all messages from the currently selected maildirs.
     */
    CGlobal *global = CGlobal::Instance();
    std::vector<CMessage *> *messages = global->get_messages();

    /**
     * How many lines we've scrolled down the message.
     */
    int offset = global->get_message_offset();

    /**
     * The number of items we've found, vs. the size of the screen.
     */
    int count = messages->size();
    int selected = global->get_selected_message();


    /**
     * Bound the selection.
     */
    if (selected >= count) {
        selected = count-1;
        global->set_selected_message(selected);
    }

    CMessage *cur = NULL;
    if (((selected) < count) && count > 0 )
        cur = messages->at(selected);
    else
    {
        move(3,3);
        printw(NO_MESSAGES);
        return;
    }

    /**
     * Now we have a message - display it.
     */

    /**
     * Find the headers we'll print.
     */
    CLua *lua = CLua::Instance();
    std::vector<std::string> headers = lua->table_to_array( "headers" );

    /**
     * If there are no values then use the defaults.
     */
    if ( headers.empty() )
    {
        headers.push_back( "$DATE" );
        headers.push_back( "$FROM" );
        headers.push_back( "$TO" );
        headers.push_back( "$SUBJECT" );
    }

    int row = 0;

    /**
     * For each header.
     */
    std::vector<std::string>::iterator it;
    for (it = headers.begin(); it != headers.end(); ++it)
    {
        move( row, 0 );

        /**
         * The header-name, in useful format - i.e. without the '$' prefix
         * and in lower-case.
         */
        std::string name = (*it);
        name = name.substr(1);
        std::transform(name.begin(), name.end(), name.begin(), tolower);

        /**
         * Upper-case first character.
         */
        name[0] = toupper(name[0]);

        /**
         * Now we've gone from "$DATE" -> "Date", etc.
         */

        /**
         * Get the header-value, via the formatter.
         */
        std::string value = cur->format( *it );

        /**
         * Truncate to avoid long-wraps.
         */
        value = value.substr(0, (CScreen::width() - name.size() - 4 ) );

        /**
         * Show it.
         */
        printw( "%s: %s", name.c_str(), value.c_str() );
        row += 1;
    }

   /**
    * Draw the attachments.
    */
    std::vector<std::string> attachments = cur->attachments();
    int acount = 1;
    for (it = attachments.begin(); it != attachments.end(); ++it)
    {
        std::string path = (*it);
        move( row, 0 );
        printw( "Attachment %d - %s", acount, path.c_str() );
        acount += 1;
        row += 1;
    }

    /**
     * Now draw the body.
     */
    std::vector<std::string> body = cur->body();

    /**
     * How many lines to draw?
     */
    int max = std::min((int)body.size(), (int)(CScreen::height() - headers.size() - attachments.size() ) );

    for( int i = 0; i <= max; i++ )
    {
        move( i + ( headers.size() + attachments.size() + 1 ), 0 );

        std::string line = "";
        if ( (i + offset) < (int)body.size() )
            line = body[i+offset];

        printw( "%s", line.c_str() );
    }

    /**
     * We're reading a message so call our hook.
     */
    lua->execute( "on_read_message(\"" + cur->path() + "\");" );
}

/**
 * Setup the curses/screen.
 */
void CScreen::setup()
{
    /**
     * Setup ncurses.
     */
    initscr();

    /**
     * Make sure we have colours.
     */
    if (!has_colors() || (start_color() != OK))
    {
	endwin();
	std::cerr << MISSING_COLOR_SUPPORT << std::endl;
	exit(1);
    }

    keypad(stdscr, TRUE);
    crmode();
    noecho();
    curs_set(0);
    timeout(1000);

    /**
     * We want (red + black) + (white + black)
     */
    init_pair(1, COLOR_RED, COLOR_BLACK);
    init_pair(2, COLOR_WHITE, COLOR_BLACK);

}

/**
 * Return the width of the screen.
 */
int CScreen::width()
{
    struct winsize w;
    ioctl(0, TIOCGWINSZ, &w);
    return (w.ws_col);
}

/**
 * Return the height of the screen.
 */
int CScreen::height()
{
    struct winsize w;
    ioctl(0, TIOCGWINSZ, &w);
    return (w.ws_row);
}

/**
 * Clear the status-line of the screen.
 */
void CScreen::clear_status()
{
    move(CScreen::height() - 1, 0);

    for (int i = 0; i < CScreen::width(); i++)
	printw(" ");

}



/**
 * Clear the main display area, leaving the status-area alone.
 */
void CScreen::clear_main()
{

    /**
     * Clear all the screen - but not the prompt.
     */
    int width = CScreen::width();
    int height = CScreen::height();

    std::string blank = "";
    while( (int)blank.length() < width )
        blank += " ";

    for(int i = 0; i < ( height - 1 ); i++ )
        mvprintw( i, 0, "%s", blank.c_str() );

}


/**
 * Given the input text return a single completion.
 *
 * NOTE: The caller must free the returned string.
 */
char *CScreen::get_completion( const char *input, size_t size, int position )
{
    char ret[1024] = { '\0' };

    /**
     * So we have a string and we know the cursor is at position "position".
     *
     * We want to walk backwards until we find the preceeding space/tab/( to
     * know where to expand from.
     */
    const char *p = input + position - 1;

    while ( ( p[0] != ' ' ) &&
            ( p[0] != '\\' ) &&
            ( p[0] != '(' ) &&
            ( p[0] != '"' ) &&
            ( p[0] != '\'' ) &&
            ( p >= input ) )
        p -=1;


    /**
     * Ensure we didn't go too far.
     */
    if ( p < input )
        p = input;
    else
        p += 1;

    /**
     * Now we have the input buffer and a sized string to remove.
     *
     * We want to replace whatever is between "p" and "start+size".
     *
     */
    size_t span = size - ( p - input );


    /**
     * tilde expansion.
     */
    if (  ( strncmp( p, "~", 1 ) == 0 ) &&
          ( getenv( "HOME") != NULL ) )
    {
        /**
         * Copy the leading section of the input.
         */
        strncpy(ret,input, p-input );

        /**
         * Add the extra section.
         */
        strcat(ret, getenv( "HOME" ) );
        return( strdup( ret ) );
    }


    /**
     * Primitive expansion.
     */
    for( int i = 0; i < primitive_count ; i ++ )
    {
        if( strncmp( p, primitive_list[i].name, span ) == 0 )
        {
            /**
             * Copy the leading section of the input.
             */
            strncpy(ret,input, p-input );

            /**
             * Add the extra section.
             */
            strcat(ret, primitive_list[i].name );
            strncat(ret, "(", 1 );
            return( strdup( ret ) );
        }
    }

    /**
     * See if we can complete on a user-called function.
     */
    CLua *lua = CLua::Instance();
    std::vector<std::string> f = lua->on_complete();
    if ( f.size() > 0 )
    {
        std::vector<std::string>::iterator it;

        for (it = f.begin(); it != f.end(); ++it)
        {
            if( strncmp( p, (*it).c_str(), span ) == 0 )
            {
                /**
                 * Copy the leading section of the input.
                 */
                strncpy(ret,input, p-input );

                /**
                 * Add the extra section.
                 */
                strcat(ret, (*it).c_str());
                return( strdup( ret ) );
            }
        }
    }

    /**
     * File/Path expansion.  This is a little hairy.
     */
    if ( strncmp( p, "/", 1 ) == 0 )
    {
        /**
         * Split the string into "dir/file"
         */
        char dir[1024] = { '\0' };
        char fil[1024] = { '\0' };

        /**
         * The right-most "/" will be used to split the path.
         */
        const char *slash = strrchr( p, '/' );
        if ( slash == NULL )
            return NULL;

        /**
         * Copy the directory part.
         */
        strncpy(dir,p, slash-p+1);

        /**
         * Copy the file part - this is broken for reasons I don't understand.
         */
        strncpy(fil, slash+1 , strlen(slash)-2);

        /**
         * Fixup the broken-trailing whitespace.
         */
        for( int i = 0; i < (int)strlen(fil); i ++ )
        {
            if ( ( fil[i] == ' ' ) || ( fil[i] == '\t' ) )
                fil[i] = '\0';
        }


        /**
         * Open the directory.
         */
        DIR *dp = opendir(dir);
        if ( dp == NULL )
            return NULL;


        while (true)
        {
            dirent *de = readdir(dp);
            if (de == NULL)
                break;

            if( strncmp( fil, de->d_name, strlen(fil) ) == 0 )
            {
                closedir(dp);

                /**
                 * Copy the leading section of the input.
                 */
                strncpy(ret,input, p-input );

                /**
                 * Add the extra section, the dir-name which is fixed.
                 */
                strcat(ret, dir);

                /**
                 * And the path we've found via readdir.
                 */
                strcat(ret, de->d_name );
                return( strdup( ret ) );
            }
        }
        closedir(dp);
    }


    /**
     * No match.
     */
    return NULL;
}



/* Read up to buflen-1 characters into `buffer`.
 * A terminating '\0' character is added after the input.  */
void CScreen::readline(char *buffer, int buflen)
{
    /**
     * Set the cursor to be visible.
     */
    int old_curs = curs_set(1);
    int pos = 0;
    int len = 0;
    int x, y;


    /**
     * Offset into history.
     */
    CHistory *hist = CHistory::Instance();
    int hoff = hist->size();


    getyx(stdscr, y, x);
    for (;;)
    {
        int c;

        buffer[len] = ' ';
        mvaddnstr(y, x, buffer, len+1);

        /**
         * Clear the line.
         */
        for( int padding = len; padding < CScreen::width(); padding++ )
            printw(" ");

        move(y, x+pos);
        c = getch();

        if (c == KEY_ENTER || c == '\n' || c == '\r')
        {
            break;
        }
        else if (isprint(c))
        {
            if (pos < buflen-1)
            {
                memmove(buffer+pos+1, buffer+pos, len-pos);
                buffer[pos++] = c;
                len += 1;
            }
            else
            {
                beep();
            }
        }
        else if ( c == '\t' ) /* TAB-completion */
        {
            if ( ( len > 0 ) && ( pos > 0 ) )
            {
                /**
                 * Handle the expansion..
                 */
                char *reply = get_completion( buffer, len, pos );
                if ( reply != NULL )
                {
                    strcpy( buffer, reply );
                    pos = strlen(reply );
                    len = pos;
                    free( reply );
                }
            }
        }
        else if (c == 1 )   /* ctrl-a : beginning of line*/
        {
            pos = 0;
        }
        else if (c == 5 ) /* ctrl-e: end of line */
        {
            pos = len;
        }
        else if (c == 11 )  /* ctrl-k: kill to end of line */
        {
            len = pos;
        }
        else if ( ( c == 2 ) ||    /* ctrl-b : back char */
                  (c == KEY_LEFT) )
        {
            if (pos > 0)
                pos -= 1;
            else
                beep();
        }
        else if ( c == KEY_UP )
        {
            hoff -= 1;
            if ( hoff >= 0 )
            {
                std::string ent = hist->at( hoff );
                strcpy( buffer, ent.c_str() );
                pos = len = strlen(ent.c_str());
            }
            else
            {
                hoff = 0;
                beep();
            }
        }
        else if ( c == KEY_DOWN )
        {
            hoff += 1;
            if ( hoff < hist->size() )
            {
                std::string ent = hist->at( hoff );
                strcpy( buffer, ent.c_str() );
                pos = len = strlen(ent.c_str());
            }
            else
            {
                hoff = hist->size();
                beep();
            }
        }
        else if ( ( c == 6 ) || /* ctrl-f: forward char */
                  (c == KEY_RIGHT) )
        {
            if (pos < len)
                pos += 1;
            else
                beep();
        }
        else if (c == KEY_BACKSPACE)
        {
            if (pos > 0)
            {
                memmove(buffer+pos-1, buffer+pos, len-pos);
                pos -= 1;
                len -= 1;
            }
            else
            {
                beep();
            }
        }
        else if (c == KEY_DC)
        {
            if (pos < len)
            {
                memmove(buffer+pos, buffer+pos+1, len-pos-1);
                len -= 1;
            }
            else
            {
                beep();
            }
        }
        else
        {
            beep();
        }
    }

    /**
     * Ensure the string is terminated.
     */
    buffer[len] = '\0';

    if (old_curs != ERR)
        curs_set(old_curs);

    hist->add( buffer );
}
