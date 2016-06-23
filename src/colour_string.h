/*
 * colour_string.h - Utility for parsing coloured strings.
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


/*
 * Only include this header one time.
 */
#pragma once

#include <string>
#include <vector>




/**
 * The output function `draw_text_lines` is used to draw lines of text upon
 * the screen, and this function is capable of performing colour output
 * given input containing special markup.
 *
 * Colours are specified via the `$[COLOUR]` prefix, and the specified colour
 * persists until the end of the line.  The following, for example, will draw
 * a line of text with two colours:
 *
 * <code>$[RED]This is red$[YELLOW]This is yellow.</code>
 *
 * Internally the line of text is parsed into segments of text, each of
 * which contains:
 *
 * * The colour to draw
 * * The text to draw.
 *
 * This structure is used to hold that result.
 */
typedef struct _COLOUR_STRING
{
    /**
     * The colour to use for this segment.
     */
    std::string *colour;

    /**
     * The text to draw for this segment.
     */
    std::string *string;

} COLOUR_STRING;



class CColourString
{
public:

    /**
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
    static std::vector<COLOUR_STRING *> parse_coloured_string(std::string input, int offset);


};
