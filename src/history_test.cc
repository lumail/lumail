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



//
// TODO: Implement CFile::size()
//
// Then test the history persistance.
//


CuSuite *
history_getsuite()
{
    CuSuite *suite = CuSuiteNew();
    SUITE_ADD_TEST(suite, TestHistory);
    return suite;
}
