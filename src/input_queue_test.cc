/*
 * input_queue_test.cc - Test-cases for our CInputQueue class.
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

#include "input_queue.h"
#include "CuTest.h"


/**
 * Test CInputQueue
 */
void TestInputQueue(CuTest * tc)
{
    /*
     * Get the queue
     */
    CInputQueue *input = CInputQueue::instance();
    CuAssertPtrNotNull(tc, input);


    /*
     * The queue should be empty
     */
    CuAssertTrue(tc, ! input->has_pending_input());

    /*
     * Add some test-data
     */
    std::string faux = "steve";
    input->add_input(faux);

    for (int i = 0; i < (int)faux.length(); i++)
    {
        CuAssertTrue(tc, input->has_pending_input());
        char c = input->get_input();

        CuAssertTrue(tc, c == faux.at(i));
    }

    /*
     * Now we've stuffed & drained we should find our queue is empty.
     */
    CuAssertTrue(tc, !input->has_pending_input());
}


CuSuite *
input_queue_getsuite()
{
    CuSuite *suite = CuSuiteNew();
    SUITE_ADD_TEST(suite, TestInputQueue);
    return suite;
}
