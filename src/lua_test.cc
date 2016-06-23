/*
 * lua_test.cc - Test-cases for our CLua class.
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

#include "lua.h"
#include "CuTest.h"


/**
 * Test CLua
 */
void TestLua(CuTest * tc)
{
    /*
     * Get the singleton
     */
    CLua *instance = CLua::instance();
    CuAssertPtrNotNull(tc, instance);

    /*
     * The variable should be empty
     */
    std::string var = instance->get_variable("moi");
    CuAssertStrEquals(tc, var.c_str(), "");

    /*
     * Now set the variable, and test it worked.
     */
    instance->execute("moi = 'kissa'");
    var = instance->get_variable("moi");
    CuAssertStrEquals(tc, var.c_str(), "kissa");
}


/**
 * Test error-handler
 */
void TestErrorHandler(CuTest * tc)
{
    /*
     * Get the singleton
     */
    CLua *instance = CLua::instance();
    CuAssertPtrNotNull(tc, instance);

    /*
     * Define an error-handler which will set the global
     * variable "triggered" to the date/time when an error
     * was raised.
     */
    instance->execute("function on_error(msg) triggered = os.date() end");

    /*
     * The variable the error-handler should be empty, because
     * no error has yet occurred.
     */
    std::string var = instance->get_variable("triggered");
    CuAssertStrEquals(tc, var.c_str(), "");

    /*
     * Execute some bogus lua, which should trigger our error-handler,
     * which in turn will set the global 'triggered' variable.
     */
    instance->execute("function() end");

    /*
     * Now ensure that worked.
     */
    var = instance->get_variable("triggered");
    CuAssertTrue(tc, ! var.empty());

    /*
     * A date/time should always contain ":".
     */
    CuAssertTrue(tc, (strstr(var.c_str(), ":") != NULL));
}


CuSuite *
lua_getsuite()
{
    CuSuite *suite = CuSuiteNew();
    SUITE_ADD_TEST(suite, TestLua);
    SUITE_ADD_TEST(suite, TestErrorHandler);
    return suite;
}
