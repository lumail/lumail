/**
 * screen.cc - Utility functions related to the screen.
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

#define _XOPEN_SOURCE_EXTENDED 1

#include <algorithm>
#include <cctype>
#include <cstdlib>
#include <cursesw.h>
#include <dirent.h>
#include <iostream>
#include <string.h>
#include <sys/ioctl.h>

#include "debug.h"
#include "file.h"
#include "global.h"
#include "history.h"
#include "input.h"
#include "lang.h"
#include "lua.h"
#include "maildir.h"
#include "message.h"
#include "screen.h"
#include "utfstring.h"
#include "util.h"


#ifndef DEFAULT_UNREAD_COLOUR
# define DEFAULT_UNREAD_COLOUR "red"
#endif


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
    std::string *s = global->get_variable("global_mode");
    assert( s != NULL );


    if ( *s == "maildir" )
        drawMaildir();
    else if ( *s == "index" )
        drawIndex();
    else if ( *s == "message" )
        drawMessage();
    else if ( *s == "text" )
        drawText();
    else
    {
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
    std::vector<CMaildir *> display = global->get_folders();
    std::string *limit = global->get_variable("maildir_limit");

    /**
     * The colour for unread maildirs.
     */
    std::string *unread = global->get_variable( "unread_maildir_colour" );
    std::string unread_colour;
    if ( unread != NULL )
        unread_colour = *unread;
    else
        unread_colour = DEFAULT_UNREAD_COLOUR;

    /**
     * get the higlighting mode for the current column
     */
    std::string *highlight = global->get_variable( "maildir_highlight_mode");
    int highlight_mode     = lookup_curses_attribute( highlight );


    /**
     * The number of items we've found, vs. the size of the screen.
     */
    int count = display.size();
    int height = CScreen::height();

    /**
     * Correct for the status bar and that counting starts at 0
     */
    int middle = (height-2)/2;
    int selected = global->get_selected_folder();

    int rowToHighlight = 0;
    vectorPosition topBottomOrMiddle = NONE;

    /**
     * BUGFIX: When the maildir_limit changes the previous position
     * might be invalid.  This fix of resetting things ensures we're OK.
     *
     * NOTE: We use ">=" because the Maildir offset starts at zero.
     */
    if ( ( selected >= count ) || ( selected < 0 ) )
    {
        selected = 0;
        global->set_selected_folder( selected );
    }

    /**
     * default to TOP if our list is shorter then the screen height
     */
    if (selected < middle || count<=height-2)
    {
        rowToHighlight = selected;

        if ( selected >= count )
            rowToHighlight = selected = 0;


        topBottomOrMiddle = TOP;
        /**
         * if height is uneven we have to switch to the BOTTOM case on row earlier
         */
    }
    else if (  (count - selected <= middle) || (height%2==1 &&count-selected<=middle+1))
    {
        rowToHighlight =  height - count+selected-1 ;
        topBottomOrMiddle = BOTTOM;
    }
    else
    {
        rowToHighlight = middle;
        topBottomOrMiddle = MIDDLE;
    }

    /**
     * If we have no messages report that.
     */
    if ( count < 1 )
    {
        move(2, 2);
        printw("No maildirs found matching the limit '%s'.", limit->c_str());
        return;
    }

    int row = 0;

    /**
     * Selected folders.
     */
    std::vector < std::string > sfolders = global->get_selected_folders();

    for (row = 0; row < (height - 1); row++)
    {
        int unread = 0;

        move(row, 0);
        printw("  ");

        /**
         * The current object.
         */
        CMaildir *cur = NULL;
        int mailIndex=count;
        if (topBottomOrMiddle == TOP)
        {
            /**
             * we start at the top of the list so just use row
             */
            mailIndex = row;
        }
        else if (topBottomOrMiddle == BOTTOM)
        {
            /**
             * when we reached the end of the list mailIndex can maximally be
             * count-1, that this is given can easily be shown
             * row:=height-2 -> count-height+row+1 = count-height+height-2+1 = count-1
             */
            mailIndex = count-height+row+1;
        }
        else if (topBottomOrMiddle == MIDDLE)
        {
            mailIndex = row + selected - middle;
        }

        if ( (mailIndex < count) &&
             (mailIndex < (int)display.size() ) )
        {
            cur = display.at(mailIndex);
            unread = cur->unread_messages();
        }

        /**
         * Is this folder part of our selected set?
         */
        bool selectedSet = false;
        if (cur != NULL)
        {
            if (std::find(sfolders.begin(), sfolders.end(), cur->path()) != sfolders.end())
                selectedSet = true;
        }

        if (row == rowToHighlight)
            attrset(highlight_mode);

        /**
         * The item we'll draw for this row.
         */
        UTFString display = "";

        /**
         * Format.
         */
        if ( cur != NULL )
            display = cur->format( selectedSet );

        /**
         * Overwrite the full length.
         */
        if ((int)display.size() < (CScreen::width() - 3))
            display += UTFString((CScreen::width() - 3)-(int)display.size(),' ');

        move(row, 2);

        if ( unread )
        {
            if ( row == rowToHighlight )
                attron( highlight_mode | COLOR_PAIR(m_colours[unread_colour]) );
            else
                attron( COLOR_PAIR(m_colours[unread_colour]) );
        }
        printw("%s", display.c_str());

        /**
         * Reset the colours.
         */
        attrset(A_NORMAL);
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
    std::string *filter = global->get_variable("index_limit" );


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

        if ( ( filter != NULL ) &&
             ( *filter == "all" ) )
            printw( NO_MESSAGES_MATCHING_FILTER, filter->c_str() );
        else
            printw( NO_MESSAGES_IN_FOLDERS );

        int height = CScreen::height();
        int row = 4;

        for (std::string folder: folders)
        {
            /**
             * Avoid drawing into the status area.
             */
            if ( row >= (height-1) )
                break;
            /**
             * Show the name of the folder.
             */
            move( row, 5 );
            printw("%s", folder.c_str() );
            row+=1;
        }
        return;
    }

    /**
     * The colour for unread maildirs.
     */
    std::string *unread = global->get_variable( "unread_message_colour" );
    std::string unread_colour;
    if ( unread != NULL )
        unread_colour = *unread;
    else
        unread_colour = DEFAULT_UNREAD_COLOUR;

    /**
     * get the higlighting mode for the current column
     */
    std::string *highlight = global->get_variable( "index_highlight_mode");
    int highlight_mode = lookup_curses_attribute( highlight );

    /**
     * The number of items we've found, vs. the size of the screen.
     */
    int count = messages->size();
    int height = CScreen::height();
    int selected = global->get_selected_message();

    /**
     * Ensure we highlight the correct line.
     */
    if ( selected >= count )
        selected = 0;

    /**
     *  correct for the status bar and that counting starts at 1
     */
    int middle = (height-2)/2;
    int rowToHighlight = 0;
    vectorPosition topBottomOrMiddle = NONE;

    /**
     * default to TOP if our list is shorter then the screen height
     */
    if (selected < middle || count<=height-2)
    {
        topBottomOrMiddle = TOP;
        rowToHighlight = selected;
        /**
         * if height is uneven we have to switch to the BOTTOM case on row earlier
         */
    }
    else if (  (count - selected <= middle) || (height%2==1 &&count-selected<=middle+1))
    {
        topBottomOrMiddle = BOTTOM;
        rowToHighlight =  height - count+selected-1 ;
    }
    else
    {
        topBottomOrMiddle = MIDDLE;
        rowToHighlight = middle;
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
        UTFString buf;

        /**
         * The current object.
         */
        CMessage *cur = NULL;
        int mailIndex=count;
        if (topBottomOrMiddle == TOP)
        {
            /**
             * we start at the top of the list so just use row
             */
            mailIndex = row;
        }
        else if (topBottomOrMiddle == BOTTOM)
        {
            /**
             * when we reached the end of the list mailIndex can maximally be
             * count-1, that this is given can easily be shown
             * row:=height-2 -> count-height+row+1 = count-height+height-2+1 = count-1
             */
            mailIndex = count-height+row+1;
        }
        else if (topBottomOrMiddle == MIDDLE)
        {
            mailIndex = row + selected - middle;
        }

        if ( (mailIndex < count) && ( mailIndex < (int)messages->size()) )
            cur = messages->at(mailIndex);

        if (row == rowToHighlight)
            attrset(highlight_mode);

        /**
         * Is this message new/unread?
         */
        bool unread = false;
        if ( cur != NULL )
            unread = cur->is_new();

        if (unread)
        {
            if ( row == rowToHighlight )
                attron( highlight_mode | COLOR_PAIR(m_colours[unread_colour]) );
            else
                attron( COLOR_PAIR(m_colours[unread_colour]) );
        }

        if (cur != NULL)
            buf =  cur->format();

        /**
         * Pad.
         */
        if ((int)buf.size() < (CScreen::width() - 3))
            buf += UTFString((CScreen::width() - 3)-(int)buf.size(),' ');

        /**
         * Truncate.
         */
        if ((int)buf.size() > (CScreen::width() - 3))
            buf.resize((CScreen::width() - 3));

        move(row, 2);
        printw("%s", buf.c_str());

        attrset( COLOR_PAIR(m_colours[ "white"]));

        /**
         * Remove the inverse.
         */
        attrset(A_NORMAL);
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
    if (selected >= count)
    {
        selected = count-1;
        global->set_selected_message(selected);
    }
    if ( selected < 0 )
    {
        selected = 0;
        global->set_selected_message( selected );
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
        headers.push_back( "$CC" );
        headers.push_back( "$SUBJECT" );
    }

    /**
     * Get the colour to draw the headers in.
     */
    std::string *h_colour = global->get_variable( "header_colour" );
    std::string header_colour;
    if ( h_colour != NULL )
        header_colour = *h_colour;
    else
        header_colour = "white";

    int row = 0;

    /**
     * For each header.
     */
    for (std::string header : headers)
    {
        move( row, 0 );

        /**
         * The header-name, in useful format - i.e. without the '$' prefix
         * and in lower-case.
         */
        UTFString name = header;
        name = name.substr(1).lowercase();

        /**
         * Upper-case first character.
         */
        name = name.substr(0, 1).uppercase() + name.substr(1);

        /**
         * Now we've gone from "$DATE" -> "Date", etc.
         */

        /**
         * Get the header-value, via the formatter.
         */
        UTFString value = cur->format( header );

        /**
         * Truncate to avoid long-wraps.
         */
        value = value.substr(0, (CScreen::width() - name.size() - 4 ) );

        /**
         * Show it.
         */
        attrset( COLOR_PAIR(m_colours[header_colour]) );
        printw( "%s: %s", name.c_str(), value.c_str() );
        attrset( COLOR_PAIR(m_colours["white"]) );
        row += 1;
    }

    /**
     * Get the colour to draw the attachments in.
     */
    std::string *a_colour = global->get_variable( "attachment_colour" );
    std::string attachment_colour;
    if ( a_colour != NULL )
        attachment_colour = *a_colour;
    else
        attachment_colour = "white";

    /**
     * Draw the attachments.
     */
    std::vector<std::string> attachments = cur->attachments();
    int acount = 1;
    for (std::string path : attachments)
    {
        move( row, 0 );

        /**
         * Change to the right colour, draw the message,
         * and revert.
         */
        attrset( COLOR_PAIR(m_colours[attachment_colour]) );
        printw( "Attachment %d - %s", acount, path.c_str() );
        attrset( COLOR_PAIR(m_colours["white"]));

        acount += 1;
        row += 1;
    }

    /**
     * Now draw the body.
     */
    std::vector<UTFString> body;

    /**
     * The body might come from on_get_body.
     */
    body = lua->on_get_body();
    if ( body.empty() )
        body = cur->body();


    int textspace = (int)(CScreen::height() - headers.size() - attachments.size() - 1 );
    if (textspace < 2)
        textspace = 2;

    /**
     * get the body-colour
     */
    std::string *b_colour = global->get_variable( "body_colour" );
    std::string body_colour;
    if ( b_colour != NULL )
        body_colour = *b_colour;
    else
        body_colour = "white";

    /**
     * The screen width.
     */
    size_t width = CScreen::width();

    bool wrap = lua->get_bool("wrap_lines");

    /**
     * Draw each line of the body.
     */
    for( int row_idx = 0, line_idx = 0;;)
    {
        UTFString line = "";

        /** Get current line */
        if ( (line_idx + offset) < (int)body.size() )
        {
            line = body[line_idx+offset];
            line_idx++;
        }
        else
            break;

        int len = line.length();
        int slen = 0;
        UTFString subline = line.substr(0, width);

        for(;;)
        {
            move( row_idx + ( headers.size() + attachments.size() + 1 ), 0 );

            attrset( COLOR_PAIR(m_colours[body_colour]) );
            printw( "%s", subline.c_str() );
            attrset( COLOR_PAIR(m_colours["white"]) );

            row_idx++;

            slen += subline.length();

            /**
             * Should we stop displaying this line ?
             */
            if (row_idx > (textspace-2) || !wrap || len == slen)
                break;

            /**
             * Make sure we don't overstep our bounds by trying to draw past
             * the line-buffer.
             */
            if ( slen > (int)line.size() )
                break;

            subline = line.substr(slen, width );
        }

        /**
         * No screen space left
         */
        if (row_idx > (textspace-2))
             break;
    }


    /**
     * We're reading a message so call the on_read_message() hook.
     */
    cur->on_read_message();
}



/**
 * Draw text which has been set from lua.
 */
void CScreen::drawText()
{
    CGlobal *global = CGlobal::Instance();

    int offset                  = global->get_text_offset();
    std::vector<UTFString> text = global->get_text();


    /**
     * Get the (default) colour for the text.
     */
    std::string *t_colour = global->get_variable( "text_colour" );
    std::string text_colour;
    if ( t_colour != NULL )
        text_colour = *t_colour;
    else
        text_colour = "white";



    if ( offset < (int)text.size() )
    {
        int max = CScreen::height();

        for( int i = 0; i< max; i++ )
        {
            /**
             * Get the line.
             */
            UTFString line = "";
            if ( i+offset < (int)text.size() )
                line = text.at(i+offset);

            /**
             * Move to the start of the line, and setup the default colour.
             */
            move(i, 0);
            attron( COLOR_PAIR(m_colours[text_colour]) );

            /**
             * If the line is of the form ${XX YY ZZ} then strip
             * that prefix out of the way, and process it as colour/formatting.
             */
            if ( line.length() > 3 )
            {
                if ( ( line[0] == '$' ) && ( line[1] == '{' ) )
                {
                    size_t offset = line.find_first_of( '}' );
                    if ( offset != std::string::npos )
                    {
                        /**
                         * Get the code which is the start of the line
                         * minus the leading "${" and trailing "}".
                         */
                        UTFString code = line.substr(0, offset );
                        code = code.substr(2);

                        /**
                         * Now the line is just the text to display.
                         */
                        line = line.substr( offset+1 );

                        /**
                         * OK so we now need to process the code.  Given
                         * the input "${colour:pink bold} .." we'll have
                         * a code that reads "colour:pink bold"
                         *
                         * We need to split that up on whitespace and
                         * parse each part, ignoring anything we don't
                         * understand.
                         *
                         */
                        std::vector<UTFString> elems = split(code, ' ');

                        /**
                         * Now we've tokenized look for colours/bold/etc.
                         */
                        for (UTFString token : elems)
                        {
                            if ( strcasestr( token.c_str(), "colour:" ) != NULL )
                            {
                                UTFString col = token.substr( 7 );
                                attron( COLOR_PAIR(m_colours[col]) );
                            }
                            else
                            {
                                std::string *x = new std::string( token );

                                int val = lookup_curses_attribute( x, 0 );
                                if ( val != 0 )
                                    attron( val );

                                delete( x );
                            }
                        }
                    }
                }
            }
            /**
             * Draw the line of text, and reset attributes.
             */
            printw(line.c_str());
            attrset( 0 );
        }
    }
    else
    {
#ifdef LUMAIL_DEBUG
        move(3, 3);
        printw("We're outside our array of text?" );
#endif
    }
}


/**
 * Setup the curses/screen.
 */
void CScreen::setup()
{
    /**
     * Setup locale.
     */
    setlocale(LC_CTYPE, "" );
    setlocale(LC_ALL, "" );

    char e[] = "ESCDELAY=0";
    putenv( e );

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
    m_colours[ "red"  ] = 1;

    init_pair(2, COLOR_WHITE, COLOR_BLACK);
    m_colours[ "white" ] = 2;

    init_pair(3, COLOR_BLUE, COLOR_BLACK);
    m_colours[ "blue" ] = 3;

    init_pair(4, COLOR_GREEN, COLOR_BLACK);
    m_colours[ "green" ] = 4;

    init_pair(5, COLOR_CYAN, COLOR_BLACK);
    m_colours[ "cyan" ] = 5;

    init_pair(6, COLOR_MAGENTA, COLOR_BLACK);
    m_colours[ "magenta" ] = 6;

    init_pair(7, COLOR_YELLOW, COLOR_BLACK);
    m_colours[ "yellow" ] = 7;

    init_pair(8, COLOR_BLACK, COLOR_WHITE);
    m_colours[ "black" ] = 8;
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
 * Some simple remapping of keyboard input.
 */
const char *CScreen::get_key_name( gunichar c, bool isKeyCode)
{
    if ( c == '\n' )
        return( "Enter" );
    if ( c == 2 )
        return( "j" );
    if ( c == 3 )
        return( "k" );
    if ( c == ' ' )
        return ( "Space" );

    const char *name;
    if (isKeyCode)
        name = keyname( c );
    else
        name = key_name( c );

    if ( name == NULL )
        return( "UnkSymbol" );
    return( name );
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

    UTFString blank = "";
    while( (int)blank.length() < width )
        blank += " ";

    for(int i = 0; i < ( height - 1 ); i++ )
        mvprintw( i, 0, "%s", blank.c_str() );

}


/**
 * Choose a single item from a small selection.
 *
 * (This is used to resolve ambiguity in TAB-completion.)
 */
std::string CScreen::choose_string( std::vector<std::string> choices )
{
    /**
     * We don't need to resolve ambiguity unless there is more than
     * one choice to choose from.
     */
    assert( choices.size() > 0 );

    /**
     * Find longest/widest entry.
     */
    size_t max = 0;

    for (std::string choice : choices)
    {
        if ( choice.size() > max )
            max = choice.size();
    }

    /**
     * Get the dimensions.
     */
    int height = CScreen::height() - 4;
    int width = CScreen::width() - 4;
    size_t cols = 1;

    WINDOW *childwin = newwin(height, width, 2, 2);
    box(childwin, 0, 0);

    /**
     * How many columns to draw?
     */
    for( int i = 1; i < 12; i++ )
    {
        if ( max < ( size_t( width ) / i) )
             cols = i;
    }

    /**
     * We'll be careful to not draw more columns than we have items.
     */
    if ( cols > choices.size() )
        cols = choices.size();


    int selected  = 0;
    bool done     = false;
    int col_width = width / cols;

    timeout(0);

    while( !done )
    {
        refresh();

        int count = 0;

        /**
         * Drawing of each item.
         */
        int x = 0;
        int y = 2;

        for (std::string choice : choices)
        {

            /**
             * Calculate the column.
             */
            x = 2 + ( ( count % cols) * col_width );
            y = 1 + ( count / cols  );


            if ( count == selected )
                wattron(childwin,A_UNDERLINE | A_STANDOUT);
            else
                wattroff(childwin,A_UNDERLINE | A_STANDOUT);

            mvwaddstr(childwin, y, x,  choice.c_str() );
            count += 1;
        }
        wrefresh(childwin);

        gunichar key;
        bool isKeyCode;
        isKeyCode = (CInput::Instance()->get_wchar(&key) == KEY_CODE_YES);

        if ( !isKeyCode && key == '\n' )
            done = true;
        if ( !isKeyCode  && key == 27 )
        {
            delwin(childwin);
            clear_main();

            timeout(1000);
            return "";
        }
        if ( !isKeyCode && key == '\t' )
        {
            selected += 1;
            if ( selected >= (int)choices.size() )
                selected = 0;
        }

        if ( isKeyCode && key == KEY_RIGHT )
        {
            selected += 1;
            if ( selected >= (int)choices.size() )
                selected = 0;
        }
        if ( isKeyCode && key == KEY_LEFT )
        {
            selected -= 1;
            if ( selected < 0 )
                selected = (int)(choices.size()-1);

        }
    }

    delwin(childwin);
    clear_main();
    timeout(1000);
    return( choices.at( selected ) );
}



/**
 * Get all possible completions for the current token.
 */
std::vector<std::string> CScreen::get_completions( std::string token )
{
    std::vector<std::string> results;

    /**
     * Should we be case insensitive?
     */
    CLua *lua = CLua::Instance();
    bool ignore_case = lua->get_bool( "ignore_case", true );


    /**
     * Attempt to match against built-in functions.
     */
    for( int i = 0; i < primitive_count ; i ++ )
    {
        if ( ignore_case )
        {
            if( strcasestr( primitive_list[i].name, token.c_str() ) != 0 )
            {
                results.push_back( primitive_list[i].name );
            }
        }
        else
        {
            if( strstr( primitive_list[i].name, token.c_str() ) != 0 )
            {
                results.push_back( primitive_list[i].name );
            }
        }
    }


    /**
     * Attempt to match against user-defined functions.
     */
    std::vector<std::string> f = lua->on_complete();
    if ( f.size() > 0 )
    {
        for (std::string val : f)
        {
            if ( ignore_case )
            {
                if( strcasestr( val.c_str(), token.c_str() ) != 0 )
                {
                    results.push_back( val );
                }
            }
            else
            {
                if ( val == token )
                {
                    results.push_back( val );
                }
            }
        }
    }

    /**
     * Avoid further processing if the token is empty.
     */
    if ( token.size() < 1 )
        return( results );

    /**
     * File/directory completion.
     */
    if ( ( token.size() > 0 ) && ( token.at(0) == '/' ) )
    {
        /**
         * Get the directory-name.
         */
        size_t offset = token.find_last_of( "/" );
        if ( offset != std::string::npos )
        {
            std::string dir = token.substr(0,offset);

            /**
             * Ensure we have a trailing "/".
             */
            if ( ( dir.empty() ) ||
                 ( !dir.empty() && ( dir.rbegin()[0] != '/' ) ) )
                dir += "/";


            std::string file = token.substr(offset+1);

            /**
             * Open the directory.
             */
            DIR *dp = opendir(dir.c_str());
            if ( dp != NULL )
            {
                while (true)
                {
                    dirent *de = readdir(dp);
                    if (de == NULL)
                        break;

                    /**
                     * Skip dots..
                     */
                    if ( ( strcmp( de->d_name, "." ) != 0 ) &&
                         ( strcmp( de->d_name, ".." ) != 0 ) &&
                         ( strncasecmp( file.c_str(), de->d_name, file.size() ) == 0 ) )
                    {

                        /**
                         * If completing a directory add the trailing "/"
                         * automatically.
                         */
                        std::string option = dir + de->d_name ;
                        if ( CFile::is_directory( option ) )
                            option += "/";

                        results.push_back( option );
                    }
                }
                closedir(dp);
            }

        }
    }


    /**
     * Tilde expansion.
     *
     * Here we cover two cases:
     *
     *  The token is "~"
     *  The token is "~/somthing.."
     *
     */
    if ( token.at(0) == '~' )
    {
        std::string home = getenv( "HOME" );

        if ( !home.empty() )
        {
            if ( token.size() == 1 )
            {
                results.push_back( home + "/" );
            }
            else
            {
                std::string path = token.substr(1);
                results.push_back( home + path );
            }
        }
    }


    /**
     * Complete on the names of environmental variables.
     */
    if ( token.at(0) == '$' )
    {
        /**
         * Skip the leading "$".
         */
        token = token.substr(1);

        /**
         * Iterate over environmental variables.
         */
        extern char **environ;
        for (char **env = environ; *env; ++env)
        {
            /**
             * Split by the "=" and complete the name.
             */
            std::string e(*env);
            size_t off = e.find( "=" );

            if ( off !=std::string::npos )
            {
                e = e.substr(0,off);

                if ( ignore_case )
                {

                    if( strncasecmp( e.c_str(), token.c_str(), token.size() ) == 0 )
                        results.push_back( "$" + e );
                }
                else
                {
                    if( strncmp( e.c_str(), token.c_str(), token.size() ) == 0 )
                        results.push_back( "$" + e );
                }
            }
        }

        /**
         * Reset the token, in case there are later completions
         * beneath this code.
         */
        token = "$" + token;
    }

    /**
     * Remove any duplicates.
     */
    sort( results.begin(), results.end() );
    results.erase( unique( results.begin(), results.end() ), results.end() );


    return( results );
}



/**
 * Read a line of input from the user.
 */
UTFString CScreen::get_line()
{
    UTFString buffer;

    int old_curs = curs_set(1);
    int pos = 0;
    int x, y;


    /**
     * Offset into history.
     */
    CHistory *history = CHistory::Instance();
    int hoff          = history->size();


    /**
     * Get completion characters to split input on.
     */
    CGlobal *global    = CGlobal::Instance();
    std::string *split = global->get_variable( "completion_chars" );


    /**
     * Get the cursor position
     */
    getyx(stdscr, y, x);

    while( true )
    {
        gunichar c;
        bool isKeyCode;

        mvaddnstr(y, x, buffer.c_str(), buffer.bytes());

        /**
         * Clear the line.
         */
        for( int padding = buffer.size(); padding < CScreen::width(); padding++ )
            printw(" ");

        /**
         * Move the cursor
         */
        move(y, x+pos);

        /**
         * Get input - paying attention to the buffer set by 'stuff()'.
         */
        isKeyCode = (CInput::Instance()->get_wchar(&c) == KEY_CODE_YES);

        /**
         * Ropy input-handler.
         */
        if ( (isKeyCode && c == KEY_ENTER) || (c == '\n' || c == '\r')  )
        {
            break;
        }
        else if (c == 1 )   /* ctrl-a : beginning of line*/
        {
            pos = 0;
        }
        else if (c == 5 ) /* ctrl-e: end of line */
        {
            pos = buffer.size();
        }
        else if (c == 11 )  /* ctrl-k: kill to end of line */
        {
            /**
             * Kill the buffer from the current position onwards.
             */
            buffer = buffer.substr(0,pos);
        }
        else if ( ( c == 2 ) ||    /* ctrl-b : back char */
                  (  isKeyCode && ( c == KEY_LEFT) ) )
        {
            if (pos > 0)
                pos -= 1;
        }
        else if ( ( c == 6 ) || /* ctrl-f: forward char */
                  ( isKeyCode && ( c == KEY_RIGHT) ) )
        {
            if (pos < (int)buffer.size())
                pos += 1;
        }
        else if ( c == 18 && ( ! buffer.empty() ) )  /* ctrl-r: history-search */
        {
            /**
             * Read the current buffer, and replace it with the most recent
             * matching entry from the history.
             */
            UTFString tmp = history->matching( buffer );
            if ( !tmp.empty() )
            {
                buffer = tmp;
                pos = buffer.size();
            }


        }
        else if ( isKeyCode && (  c == KEY_UP ) )
        {
            hoff -= 1;
            if ( hoff >= 0 )
            {
                buffer = history->at( hoff );
                pos    = buffer.size();
            }
            else
            {
                hoff = 0;
            }
        }
        else if ( isKeyCode &&  ( c == KEY_DOWN ) )
        {
            hoff += 1;
            if ( hoff < history->size() )
            {
                buffer = history->at( hoff );
                pos    = buffer.size();
            }
            else
            {
                hoff = history->size();
            }
        }
        else if (isKeyCode &&  ( c == KEY_BACKSPACE))
        {
            if (pos > 0)
            {
                buffer.erase(pos-1,1);
                pos -= 1;
            }
        }
        else if (c == 4)  /* ctrl+d */
        {
            /**
             * Remove the character after the point.
             */
            if ( pos < (int)buffer.size() )
            {
                buffer.erase(pos,1);
            }
        }
        else if ( (c == '\t' ) && ( ! buffer.empty() ) ) /* TAB-completion */
        {
            /**
             * We're going to find the token to complete against
             * by searching backwards for a position to start from.
             *
             * This string comes from lua, and includes things like: ( " ' space
             *
             */
            size_t toke = buffer.find_last_of( *split, pos );

            std::string prefix = "";
            std::string token  = buffer;

            /**
             * If we found one of the split-characters then we have
             * a token to complete, and the prefix to ignore.
             *
             * If we didn't then the prefix is empty, and the buffer is
             * the token; i.e. we're completing the sole token on the line.
             *
             * NOTE:  This implies you cannot complete in the middle of a line.
             * Just at the end.  Or start.
             *
             */
            if ( toke != std::string::npos )
            {
                prefix = buffer.substr(0,toke+1);
                token = token.substr(toke+1);
            }


            /**
             * The token length - because we want to update the cursor position, post-completion.
             */
            int toke_len = token.size();

            /**
             * Get the completions.
             */
            std::vector<std::string> matches = get_completions( token );
            if ( matches.size() == 0 )
            {
                /**
                 * No completion possible.
                 */
                beep();
            }
            else
            {
                /**
                 * Single completion == match.
                 */
                if ( matches.size() == 1 )
                {
                    buffer = prefix + matches.at(0).c_str();
                    pos += ( matches.at(0).size() - toke_len );
                }
                else
                {
                    /**
                     * Disable echoing before showing the menu.
                     */
                    noecho();
                    curs_set(0);

                    /**
                     * Prompt for clarification in the multiple-matches.
                     */
                    std::string choice = choose_string( matches );

                    /**
                     * Reset the cursor.
                     */
                    curs_set(1);
                    echo();

                    /**
                     * If the user did make a specific choice, then use it.
                     */
                    if ( ! choice.empty() )
                    {
                        buffer = prefix + choice.c_str();
                        pos += ( choice.size() - toke_len );
                    }
                }
            }
        }
        else if ( (c == '\t' ) && (  buffer.empty() ) ) /* TAB-completion on empty buffer */
        {
            beep();
        }
        else if ( !isKeyCode && g_unichar_isprint(c) )
        {
            /**
             * Insert the character into the buffer-string.
             */
            buffer.insert(pos, 1, c);
            pos +=1;
        }

    }

    if (old_curs != ERR)
        curs_set(old_curs);

    /**
     * Add the input to the history.
     */
    history->add( buffer );

    return( buffer );
}


/**
 * Lookup the curses attribute for the given string.
 */
int CScreen::lookup_curses_attribute( std::string *name, int default_val )
{
    assert(name != NULL);
    assert(!name->empty());

    int result = 0;

    if (name->find("underline") != std::string::npos)
        result |= A_UNDERLINE;

    if (name->find("standout") != std::string::npos)
        result |= A_STANDOUT;

    if (name->find("reverse") != std::string::npos)
        result |= A_REVERSE;

    if (name->find("blink") != std::string::npos)
        result |= A_BLINK;

    if (name->find("dim") != std::string::npos)
        result |= A_DIM;

    if (name->find("bold") != std::string::npos)
        result |= A_BOLD;

    if ( result != 0 )
        return( result );
    else
        return( default_val );
}
