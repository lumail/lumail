/*
 * main.cc - Driver for our test-cases.
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
#include <stdio.h>


#include "CuTest.h"



/* defined in coloured_string_test.cc */
CuSuite *coloured_string_getsuite();

/* defined in util_test.cc */
CuSuite *util_getsuite();


/**
 * Run all the available tests, and report upon their results.
 */
void RunAllTests(void)
{
    CuString *output = CuStringNew();
    CuSuite *suite = CuSuiteNew();

    CuSuiteAddSuite(suite, coloured_string_getsuite());
    CuSuiteAddSuite(suite, util_getsuite());

    CuSuiteRun(suite);
    CuSuiteSummary(suite, output);
    CuSuiteDetails(suite, output);
    printf("%s\n", (char *) output->buffer);
}



/**
 * Entry point to our code.
 */
int main(int argc, char *argv[])
{
    RunAllTests();

    return 0;
}
