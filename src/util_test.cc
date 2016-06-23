/*
 * util_test.cc - Test-cases for our util-class.
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



#include <string>
#include <vector>

#include "util.h"
#include "CuTest.h"



/**
 * Helper for split-tests.
 */
typedef struct _split_test_case
{
    std::string input;
    char c;
    int expected;
} split_test_case;


/**
 * Helper for escape-tests.
 */
typedef struct _escape_test_case
{
    std::string input;
    std::string output;
} escape_test_case;



/**
 * Test our splitting function.
 */
void TestSplit(CuTest * tc)
{
    split_test_case tests[] =
    {
        {"", ';', 1},
        {"test", ';', 1},
        {"test;me", ';', 2},
        {" ", ' ', 2},
        {"   ", ' ', 4},
        {"      ", ' ', 7},
        {"          ", ' ', 11},
    };

    /*
     * Number of test-cases in the array above.
     */
    int max = sizeof(tests) / sizeof(tests[0]);
    CuAssertIntEquals(tc, 7, max);

    /*
     * Run each test
     */
    for (int i = 0; i < max; i++)
    {
        split_test_case cur = tests[i];

        std::vector<std::string> out = split(cur.input, cur.c);

        CuAssertIntEquals(tc, cur.expected, out.size());
    }

}


/**
 * Test our escape function.
 */
void TestEscape(CuTest * tc)
{
    escape_test_case tests[] =
    {
        {"", ""},
        {"foo", "foo"},
        {"/", "_" },
        {"\\", "_" },
        {"http://", "http:__"},
        {"`ls`", "`ls`"},
    };

    /*
     * Number of test-cases in the array above.
     */
    int max = sizeof(tests) / sizeof(tests[0]);
    CuAssertIntEquals(tc, 6, max);

    /*
     * Run each test
     */
    for (int i = 0; i < max; i++)
    {
        escape_test_case cur = tests[i];

        std::string out = escape_filename(cur.input);

        CuAssertStrEquals(tc, cur.output.c_str(), out.c_str());
    }

}

CuSuite *
util_getsuite()
{
    CuSuite *suite = CuSuiteNew();
    SUITE_ADD_TEST(suite, TestSplit);
    SUITE_ADD_TEST(suite, TestEscape);
    return suite;
}
