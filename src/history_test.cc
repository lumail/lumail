/*
 * history_test.cc - Test-cases for our CHistory class.
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
#include "history.h"
#include "CuTest.h"


/**
 * Test CHistory
 */
void TestHistory(CuTest * tc)
{
    /*
     * Get the singleton
     */
    CHistory *instance = CHistory::instance();
    CuAssertPtrNotNull(tc, instance);

    /*
     * History should be empty.
     */
    CuAssertIntEquals(tc, instance->size(), 0);

    /*
     * Add three entries.
     */
    std::vector<std::string> tmp;
    tmp.push_back("steve");
    tmp.push_back("kemp");
    tmp.push_back("moi");

    for (std::vector<std::string>::iterator it = tmp.begin(); it != tmp.end() ; ++it)
    {
        instance->add((*it));
    }

    /*
     * Size should be three
     */
    CuAssertIntEquals(tc, 3, instance->size());

    /*
     * Entries should be in the right order.
     */
    for (int i = 0; i < (int)tmp.size(); i++)
    {
        CuAssertStrEquals(tc, tmp.at(i).c_str(), instance->at(i).c_str());
    }

    /*
     * Remove all history, and ensure it worked.
     */
    instance->clear();
    CuAssertIntEquals(tc, 0, instance->size());
}



/**
 * Test CHistory persistence.
 */
void TestHistoryPersistence(CuTest * tc)
{
    /*
     * Get the singleton
     */
    CHistory *instance = CHistory::instance();
    CuAssertPtrNotNull(tc, instance);

    /*
     * History should be empty.
     */
    CuAssertIntEquals(tc, instance->size(), 0);

    /**
     * Create a file to save the history to.
     */
    char *t_src = strdup("srcXXXXXX");
    char *src   = tmpnam(t_src);

    /*
     * The history file shouldn't exist, which means its size will be -1.
     */
    CuAssertTrue(tc, !CFile::exists(src));
    CuAssertIntEquals(tc, CFile::size(src),-1);

    /*
     * Set the file, and write some strings.
     */
    instance->set_file( src );

    /*
     * Ensure the size increases
     */
    std::vector<std::string> tmp;
    tmp.push_back("yksi");
    tmp.push_back("kaski");
    tmp.push_back("kolme");

    int base = 0;

    for (std::vector<std::string>::iterator it = tmp.begin(); it != tmp.end() ; ++it)
    {
        std::string txt = (*it);

        /*
         * Add one line of text.
         */
        instance->add(txt);


        int size = CFile::size( src );

        /*
         * Test the size of the file is the existing size,
         * plus the added-line, plus the newline.
         */
        CuAssertIntEquals(tc, base+txt.size()+1, size );

        /*
         * Bump our base-size - also add the newline.
         */
        base += txt.size();
        base += 1;
    }

    /*
     * Now change the filename, and we should find our history
     * is empty.
     */
    instance->set_file( "bogus/path" );
    CuAssertIntEquals(tc, 0,instance->size());

    /*
     * But if we go back to the file we read we should have
     * our history, as expected.
     */
    instance->set_file( src );

    /*
     * Size should be three
     */
    CuAssertIntEquals(tc, 3, instance->size());

    /*
     * Entries should be in the right order.
     */
    for (int i = 0; i < (int)tmp.size(); i++)
    {
        CuAssertStrEquals(tc, tmp.at(i).c_str(), instance->at(i).c_str());
    }

    /*
     * Remove our file
     */
    CFile::delete_file( src );
    CuAssertTrue(tc, !CFile::exists(src));
    CuAssertIntEquals(tc, CFile::size(src),-1);
}


CuSuite *
history_getsuite()
{
    CuSuite *suite = CuSuiteNew();
    SUITE_ADD_TEST(suite, TestHistory);
    SUITE_ADD_TEST(suite, TestHistoryPersistence);
    return suite;
}
