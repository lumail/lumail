/*
 * file_test.cc - Test-cases for our CFile class.
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

#include "directory.h"
#include "file.h"
#include "CuTest.h"

/*
 * GROSS:
 */
#include "directory.cc"
#include "file.cc"


/**
 * Test CFile::exists()
 */
void TestFileExists(CuTest * tc)
{
    /**
     * Generate a temporary filename.
     */
    char *tmpl = strdup("blahXXXXXX");
    char *filename = tmpnam(tmpl);

    /**
     * Ensure it doesn't exist.
     */
    CuAssertTrue(tc, ! CFile::exists(filename));

    /*
     * Now create it.
     */
    std::fstream fs;
    fs.open(filename,  std::fstream::out | std::fstream::app);
    fs << "Gordon's alive!" << "\n";
    fs.close();


    /*
     * Ensure it exists
     */
    CuAssertTrue(tc, CFile::exists(filename));

    /*
     * Finally remove it, and confirm it is gone.
     */
    CFile::delete_file(filename);
    CuAssertTrue(tc, ! CFile::exists(filename));


}


/**
 * Test CFile::is_directory()
 */
void TestFileDirectory(CuTest * tc)
{
    /**
     * Generate a temporary filename.
     */
    char *tmpl = strdup("blahXXXXXX");
    char *filename = tmpnam(tmpl);

    /**
     * Ensure it doesn't exist.
     */
    CuAssertTrue(tc, ! CFile::exists(filename));
    CuAssertTrue(tc, ! CFile::is_directory(filename));

    /**
     * Make the directory.
     */
    CDirectory::mkdir_p(filename);

    /**
     * Ensure it exists now.
     */
    CuAssertTrue(tc, CFile::exists(filename));
    CuAssertTrue(tc, CFile::is_directory(filename));

    /**
     * Cleanup
     */
    rmdir(filename);
    CuAssertTrue(tc, !CFile::exists(filename));
    CuAssertTrue(tc, !CFile::is_directory(filename));
}


CuSuite *
file_getsuite()
{
    CuSuite *suite = CuSuiteNew();
    SUITE_ADD_TEST(suite, TestFileExists);
    SUITE_ADD_TEST(suite, TestFileDirectory);
    return suite;
}
