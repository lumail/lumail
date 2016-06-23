/*
 * colour_string_test.cc - Test-cases for our coloured string utility.
 *
 * This file is part of lumail - http://lumail.org/
 *
 * Copyright (c) 2016 by Steve Kemp.  All rights reserved.
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



#include <stdlib.h>
#include <string.h>
#include <malloc.h>

#include "colour_string.h"
#include "CuTest.h"



/**
 * Test an empty string parses to zero parts.
 */
void TestEmptyString(CuTest * tc)
{
    std::string input = "";

    std::vector<COLOUR_STRING *> parts = CColourString::parse_coloured_string(input, 0);

    CuAssertIntEquals(tc, parts.size(), 0);
}


/**
 * Test a blank string parses to one parts.
 */
void TestBlankString(CuTest * tc)
{
    std::string input = " ";
    std::vector<COLOUR_STRING *> parts = CColourString::parse_coloured_string(input, 0);

    CuAssertIntEquals(tc, parts.size(), 1);
}


/**
 * Test a single string has only one colour
 */
void TestNoColours(CuTest * tc)
{

    std::string input = "Steve Kemp";

    std::vector<COLOUR_STRING *> parts;
    parts = CColourString::parse_coloured_string(input, 0);

    /*
     * We expect one part for each character
     */
    CuAssertIntEquals(tc, strlen("Steve Kemp"), parts.size());

    /*
     * Each part will default to 'white'.
     */
    for (std::vector<COLOUR_STRING *>::iterator it = parts.begin(); it != parts.end() ; ++it)
    {
        std::string *colour = (*it)->colour;

        CuAssertStrEquals(tc, "white", colour->c_str());
    }
}


/**
 * Test a single string expands to parts of one character each.
 */
void TestStringPartLength(CuTest * tc)
{

    std::string input = "Steve Kemp";

    std::vector<COLOUR_STRING *> parts;
    parts = CColourString::parse_coloured_string(input, 0);

    /*
     * We expect one part for each character
     */
    CuAssertIntEquals(tc, strlen("Steve Kemp"), parts.size());

    /*
     * Each part should only be one byte.
     */
    for (std::vector<COLOUR_STRING *>::iterator it = parts.begin(); it != parts.end() ; ++it)
    {
        std::string *text   = (*it)->string;

        CuAssertIntEquals(tc, 1, text->length());
    }
}


/**
 * Test a multibyte string expands to characters still.
 */
void TestSimpleMultiByte(CuTest * tc)
{
    std::string input = "的展会";

    std::vector<COLOUR_STRING *> parts;
    parts = CColourString::parse_coloured_string(input, 0);

    /*
     * We expect three parts, since there are three characters.
     */
    CuAssertIntEquals(tc, 3, parts.size());

    /*
     * But the input text will still be longer.
     */
    int length = 0;

    /*
     * Add up the length of each text-part so we can see we've not dropped
     * a character.
     */
    for (std::vector<COLOUR_STRING *>::iterator it = parts.begin(); it != parts.end() ; ++it)
    {
        std::string *text = (*it)->string;
        int len           = text->length();

        length += len;

        /*
         * Because we're using multibytes we know each length
         * will be more than one character.
         */
        CuAssertTrue(tc, len > 1);
    }

    CuAssertIntEquals(tc, input.length(), length);
}

CuSuite *
coloured_string_getsuite()
{
    CuSuite *suite = CuSuiteNew();
    SUITE_ADD_TEST(suite, TestEmptyString);
    SUITE_ADD_TEST(suite, TestBlankString);
    SUITE_ADD_TEST(suite, TestNoColours);
    SUITE_ADD_TEST(suite, TestStringPartLength);
    SUITE_ADD_TEST(suite, TestSimpleMultiByte);
    return suite;
}
