/*
 * statuspanel_test.cc - Test-cases for our CStatusPanel class.
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




#include "statuspanel.h"
#include "CuTest.h"


/**
 * Test appending appending/clearing the disaply.
 */
void TestStatusPanelAppend(CuTest * tc)
{
    /**
     * The status-panel
     */
    CStatusPanel *panel = CStatusPanel::instance();

    /*
     * Get the text - we have two define lines
     */
    std::vector<std::string> lines = panel->get_text();
    CuAssertIntEquals(tc, 2, lines.size());

    /*
     * Add three more lines.
     */
    std::vector<std::string> test;
    test.push_back("This is");
    test.push_back("some text for");
    test.push_back("testing purposes");

    for (auto it = test.begin(); it != test.end() ; ++it)
    {
        panel->add_text(*(it));
    }

    /*
     * The size should be bumped.
     */
    lines = panel->get_text();
    CuAssertIntEquals(tc, 5, lines.size());

    /*
     * Empty all the lines and test it worked.
     */
    panel->reset();
    lines = panel->get_text();
    CuAssertIntEquals(tc, 0, lines.size());

    /*
     * All done.
     */
}



CuSuite *
statuspanel_getsuite()
{
    CuSuite *suite = CuSuiteNew();
    SUITE_ADD_TEST(suite, TestStatusPanelAppend);
    return suite;
}
