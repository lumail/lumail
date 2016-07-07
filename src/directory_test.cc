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
#include "file.h"
#include "CuTest.h"


/**
 * Test CDirectory::entries()
 */
void TestDirectoryEntries(CuTest * tc)
{
    /*
     * We're going to look for source files.
     */
    if (CFile::is_directory("/etc"))
    {
        std::vector<std::string> entries;
        entries = CDirectory::entries("/etc");

        /*
         * If we have /etc it should be non-empty.
         */
        CuAssertTrue(tc, (entries.size() > 0));

        /*
         * At least one file should match the pattern "host"
         */
        int count = 0;

        for (std::vector<std::string>::iterator it = entries.begin(); it != entries.end() ; ++it)
        {
            if (strstr((*it).c_str(), "host") != NULL)
            {
                count += 1;
            }
        }

        CuAssertTrue(tc, (count > 0));
    }
}


/**
 * Test CDirectory::exists()
 */
void TestDirectoryExists(CuTest * tc)
{
#ifdef DEBUG
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
    CDirectory::mkdir_p(filename);

    CuAssertTrue(tc,  CDirectory::exists(filename));

    /*
     * Remove it.
     */
    rmdir(filename);
    CuAssertTrue(tc, ! CDirectory::exists(filename));
#endif
}


/*
 * Create a temporary directory, then create "foo/bar/baz" beneath it.
 */
void TestDirectoryMkdir(CuTest * tc)
{
#ifdef DEBUG
    char *tmpl = strdup("blahXXXXXX");
    std::string prefix = tmpnam(tmpl);

    /*
     * The complete tree we want.
     */
    std::string desired = prefix +  "/foo/bar/baz";

    /*
     * None of the parts will exist.
     */
    CuAssertTrue(tc, !CDirectory::exists(prefix));
    CuAssertTrue(tc, !CDirectory::exists(prefix + "/foo"));
    CuAssertTrue(tc, !CDirectory::exists(prefix + "/foo/bar"));
    CuAssertTrue(tc, !CDirectory::exists(prefix + "/foo/bar/baz"));

    CDirectory::mkdir_p(desired);

    /*
     * Post-creation they all should.
     */
    CuAssertTrue(tc, CDirectory::exists(prefix + "/foo/bar/baz"));
    CuAssertTrue(tc, CFile::is_directory(prefix + "/foo/bar/baz"));

    CuAssertTrue(tc, CDirectory::exists(prefix + "/foo/bar"));
    CuAssertTrue(tc, CFile::is_directory(prefix + "/foo/bar"));

    CuAssertTrue(tc, CDirectory::exists(prefix + "/foo"));
    CuAssertTrue(tc, CFile::is_directory(prefix + "/foo"));

    CuAssertTrue(tc, CDirectory::exists(prefix));
    CuAssertTrue(tc, CFile::is_directory(prefix));

    /*
     * Cleanup
     */
    rmdir(std::string(prefix + "/foo/bar/baz").c_str());
    rmdir(std::string(prefix + "/foo/bar").c_str());
    rmdir(std::string(prefix + "/foo").c_str());
    rmdir(prefix.c_str());

    /*
     * None of the parts should exist now.
     */
    CuAssertTrue(tc, !CDirectory::exists(prefix));
    CuAssertTrue(tc, !CFile::is_directory(prefix));

    CuAssertTrue(tc, !CDirectory::exists(prefix + "/foo"));
    CuAssertTrue(tc, !CFile::is_directory(prefix + "/foo"));

    CuAssertTrue(tc, !CDirectory::exists(prefix + "/foo/bar"));
    CuAssertTrue(tc, !CFile::is_directory(prefix + "/foo/bar"));

    CuAssertTrue(tc, !CDirectory::exists(prefix + "/foo/bar/baz"));
    CuAssertTrue(tc, !CFile::is_directory(prefix + "/foo/bar/baz"));
#endif
}


CuSuite *
directory_getsuite()
{
    CuSuite *suite = CuSuiteNew();
    SUITE_ADD_TEST(suite, TestDirectoryEntries);
    SUITE_ADD_TEST(suite, TestDirectoryExists);
    SUITE_ADD_TEST(suite, TestDirectoryMkdir);
    return suite;
}
