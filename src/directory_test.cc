/*
 * directory_test.cc - Test-cases for our CDirectory class.
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



#include <fstream>
#include <iostream>
#include <string.h>
#include <unistd.h>

#include "directory.h"
#include "CuTest.h"


/**
 * Test CDirectory::exists()
 */
void TestDirectoryExists(CuTest * tc)
{
    /**
     * Generate a temporary filename.
     */
    char *tmpl = strdup("blahXXXXXX");
    char *filename = tmpnam(tmpl);

    /**
     * Ensure it doesn't exist.
     */
    CuAssertTrue(tc, ! CDirectory::exists(filename));

    /*
     * Make it
     */
    CDirectory::mkdir_p( filename );

    CuAssertTrue(tc,  CDirectory::exists(filename));

    /*
     * Remove it.
     */
    rmdir( filename );
    CuAssertTrue(tc, ! CDirectory::exists(filename));
}



CuSuite *
directory_getsuite()
{
    CuSuite *suite = CuSuiteNew();
    SUITE_ADD_TEST(suite, TestDirectoryExists);
    return suite;
}
