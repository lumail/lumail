/*
 * colour_string.cc - Utility for parsing coloured strings.
 *
 * This file is part of lumail - http://lumail.org/
 *
 * Copyright (c) 2015 by Steve Kemp.  All rights reserved.
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
#include <assert.h>
#include <malloc.h>
#include <pcrecpp.h>

#include "colour_string.h"



/*
 * Returns length indicated by first byte.
 *
 * This function should use a table lookup.
 */
int dsutil_utf8_charlen(const unsigned char  c)
{
    if ((c & 0xfe) == 0xfc)
    {
        return 6;
    }

    if ((c & 0xfc) == 0xf8)
    {
        return 5;
    }

    if ((c & 0xf8) == 0xf0)
    {
        return 4;
    }

    if ((c & 0xf0) == 0xe0)
    {
        return 3;
    }

    if ((c & 0xe0) == 0xc0)
    {
        return 2;
    }

    if ((c & 0x80) == 0x80)
    {
        /* INVALID */
        return 0;
    }

    return 1;
}

/*
 * Parse a string into an array of "string + colour" pairs,
 * which will be useful for drawing strings.
 *
 * The output of this routine will be an array of COLOUR_STRING
 * objects - each object will contain a colour and ONE CHARACTER
 * of text to draw.
 *
 * This needs re-emphasising:  The entries will contain one character
 * which may be displayed, even if that character might be made from
 * multiple *BYTES*.
 */
std::vector<COLOUR_STRING *> CColourString::parse_coloured_string(std::string input, int offset)
{
    /**
     * Vector we use while building.
     */
    std::vector<COLOUR_STRING *> temp;

    /**
     * Returned to the caller.
     */
    std::vector<COLOUR_STRING *> results;

    /*
     * I know this is horrid.
     *
     * NOTE: We're trying to be greedy but searching from the
     * back of the string forward.  This is definitely the simpler
     * of the approaches I trialled.
     */
    pcrecpp::RE re("^(.*)\\$\\[([#a-zA-Z|]+)\\](.*)$");

    std::string pre;
    std::string col;
    std::string txt;

    while (re.FullMatch(input, &pre, &col, &txt))
    {
        /*
         * Allocate a structure to hold this match.
         */
        COLOUR_STRING *tmp = (COLOUR_STRING *)malloc(sizeof(COLOUR_STRING));

        /*
         * Save our match away.
         */
        tmp->colour = new std::string(col);
        tmp->string = new std::string(txt);
        temp.push_back(tmp);

        input = pre;
    }

    /*
     * If input is non-empty then we have leading match.  Handle that
     * as a special case.
     */
    if (! input.empty())
    {
        /*
         * Allocate a structure to hold this match.
         */
        COLOUR_STRING *tmp = (COLOUR_STRING *)malloc(sizeof(COLOUR_STRING));
        tmp->colour = new std::string("white");
        tmp->string = new std::string(input);
        temp.push_back(tmp);
    }

    /*
     * Remember we searched backwards?  Reverse so all makes sense.
     */
    std::reverse(temp.begin(), temp.end());


    /**
     * At this point we're half done.
     *
     * We've turned this input:
     *
     *   $[RED]This is red $[BLUE]This is blue
     *
     * Into this intermediary step:
     *
     *  [colour:RED, text:"This is red "],
     *  [colour:BLUE, text:"This is blue"],
     *
     * We now need to turn that middle-step into:
     *
     *  [colour:RED, text:T]
     *  [colour:RED, text:h]
     *  [colour:RED, text:i]
     *  [colour:RED, text:s]
     *  [colour:RED, text: ]
     *  [colour:RED, text:i]
     *  [colour:RED, text:s]
     *  [colour:RED, text: ]
     *  [colour:RED, text:r]
     *  [colour:RED, text:e]
     *  [colour:RED, text:d]
     *  [colour:RED, text: ]
     *  [colour:BLUE, text:T]
     *  [colour:BLUE, text:h]
     *  [colour:BLUE, text:i]
     *  ..
     *
     * We do this because it is significantly more efficient to handle
     * scrolling when we have a set of characters, since we just remove
     * the ones to the left of the scroll-index.
     */


    /*
     * Starting/Previous colour.
     */
    std::string prev_colour = "white";

    /*
     * Iterate over the entries.
     */
    for (std::vector<COLOUR_STRING *>::iterator it = temp.begin(); it != temp.end() ; ++it)
    {
        std::string *colour = (*it)->colour;
        std::string *text   = (*it)->string;

        /**
         * Expand $[#RED]", appropriately.
         */
        if ((colour->size() > 0) && (colour->at(0) == '#'))
        {
            /* Rewrite the text - i.e escape the whole damn thing. */
            *text = "$[" + colour->substr(1) + "]" + *text;

            /* avoid using the selected colour */
            delete(colour);

            (*it)->colour = new std::string(prev_colour);
            colour        = (*it)->colour;
        }
        else
        {
            /*
             * Record the previous colour, to handle the case of a future
             * string not updating itself because it is escaped.
             */
            prev_colour = *colour;
        }


        /*
         * Copy the colour, and the one-character string.
         */
        int max = (int)text->length();

        for (int i = 0; i < max; i++)
        {
            /*
             * The "single character" we'll draw, which might
             * actually be comprised of multiple bytes.
             */
            std::string chr;

            /*
             * Get the single byte at the position.
             * We'll test this to see if it is a multi-byte chacter
             * and if it is we'll bump it up.
             */
            const char byte = text->at(i);

            /*
             * Lookup the size of the UTF-character, in bytes.
             */
            int size = dsutil_utf8_charlen(byte);

            /*
             * If that failed because the UTF-8 is invalid we're
             * gonna have to fake it.
             */
            if (size == 0)
            {
                chr = "?";
            }
            else
            {
                /*
                 * Otherwise add each byte
                 */
                for (int j = 0; j < size; j++)
                {
                    chr += text->at(i + j);
                }

                /*
                 * Bump past the bytes we've added
                 */
                i += (size - 1);
            }

            COLOUR_STRING *tmp = (COLOUR_STRING *)malloc(sizeof(COLOUR_STRING));
            tmp->colour = new std::string(*colour);
            tmp->string = new std::string(chr);
            results.push_back(tmp);

        }
    }


    /*
     * Free the array we built in the middle.
     */
    for (std::vector<COLOUR_STRING *>::iterator it = temp.begin(); it != temp.end() ; ++it)
    {
        COLOUR_STRING *i = (*it);
        delete(i->string);
        delete(i->colour);
        free(i);
    }



    /*
     * Now remove the parts that we should skip because we have
     * scrolling in effect.
     */
    for (int i = 0; i < offset && ((int)results.size() >= 1); i++)
    {
        COLOUR_STRING *x = results.at(0);
        delete(x->string);
        delete(x->colour);
        free(x);
        results.erase(results.begin(), results.begin() + 1);
    }

    /*
     * Return the results.
     */
    return results;
}
