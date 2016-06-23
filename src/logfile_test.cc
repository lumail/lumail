/*
 * logfile_test.cc - Test-cases for our CLogfile class.
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



#include <string.h>

#include "file.h"
#include "logfile.h"
#include "CuTest.h"


/**
 * Test CLogfile
 */
void TestLogfile(CuTest * tc)
{
    /*
     * Get the singleton
     */
    CLogfile *instance = CLogfile::instance();
    CuAssertPtrNotNull(tc, instance);

    /**
     * Create a file to save the logfile-entries to.
     */
    char *t_src = strdup("srcXXXXXX");
    char *src   = tmpnam(t_src);

    /*
     * The file shouldn't exist, which means its size will be -1.
     */
    CuAssertTrue(tc, !CFile::exists(src));
    CuAssertIntEquals(tc, CFile::size(src), -1);

    /*
     * Set the file, and write some strings.
     */
    instance->set_file(src);

    /*
     * Ensure the size increases
     */
    std::vector<std::string> tmp;
    tmp.push_back("neljä");
    tmp.push_back("viisi");
    tmp.push_back("kuusi");
    tmp.push_back("seitsemän");

    int base = 0;

    for (std::vector<std::string>::iterator it = tmp.begin(); it != tmp.end() ; ++it)
    {
        std::string txt = (*it);

        /*
         * Add one line of text.
         */
        instance->append(txt);


        int size = CFile::size(src);

        /*
         * Test the size of the file is the existing size,
         * plus the added-line, plus the newline.
         */
        CuAssertIntEquals(tc, base + txt.size() + 1, size);

        /*
         * Bump our base-size - also add the newline.
         */
        base += txt.size();
        base += 1;
    }


    /*
     * Remove our file
     */
    CFile::delete_file(src);
    CuAssertTrue(tc, !CFile::exists(src));
    CuAssertIntEquals(tc, CFile::size(src), -1);
}


CuSuite *
logfile_getsuite()
{
    CuSuite *suite = CuSuiteNew();
    SUITE_ADD_TEST(suite, TestLogfile);
    return suite;
}
