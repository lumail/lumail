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



/**
 * Test a Lua function returning a table works.
 */
void TestFunctionToTable(CuTest * tc)
{
    /*
     * Get the singleton
     */
    CLua *instance = CLua::instance();
    CuAssertPtrNotNull(tc, instance);

    /*
     * Define a function that returns a table.
     */
    instance->execute("function get_table() t ={} t[1] = 'bar' t[2]= 'baz' return t end ");


    /*
     * Get the results
     */
    std::vector<std::string> results = instance->function2table("get_table", "");

    CuAssertTrue(tc, ! results.empty());
    CuAssertIntEquals(tc, 2, results.size());

    /*
     * Test we got the values we expected.
     */
    CuAssertStrEquals(tc, "bar", results.at(0).c_str());
    CuAssertStrEquals(tc, "baz", results.at(1).c_str());

}


/**
 * Test a Lua function returning a table works.
 */
void TestFunctionToTableArgs(CuTest * tc)
{
    /*
     * Get the singleton
     */
    CLua *instance = CLua::instance();
    CuAssertPtrNotNull(tc, instance);

    /*
     * Define a function that returns a table.
     */
    instance->execute("function get_table(a) t ={} if ( a == 'foo' ) then t[1] = 'bar' t[2]= 'baz' else t[1] = 'ok, computer'  end return t end ");


    /*
     * Get the results: calling with "foo"
     */
    std::vector<std::string> results = instance->function2table("get_table", "foo");

    CuAssertTrue(tc, ! results.empty());
    CuAssertIntEquals(tc, 2, results.size());

    /*
     * Test we got the values we expected.
     */
    CuAssertStrEquals(tc, "bar", results.at(0).c_str());
    CuAssertStrEquals(tc, "baz", results.at(1).c_str());



    /*
     * Get the results: Calling with "bogus"
     */
    results = instance->function2table("get_table", "bogus");

    CuAssertTrue(tc, ! results.empty());
    CuAssertIntEquals(tc, 1, results.size());

    /*
     * Test we got the values we expected.
     */
    CuAssertStrEquals(tc, "ok, computer", results.at(0).c_str());
}


/**
 * Test function detection works.
 */
void TestFunctionExists(CuTest * tc)
{
    /*
     * Get the singleton
     */
    CLua *instance = CLua::instance();
    CuAssertPtrNotNull(tc, instance);

    /*
     * The functon will not exist to start with.
     */
    CuAssertTrue(tc, instance->function_exists("hello") == false);

    /*
     * Define the function.
     */
    instance->execute("function hello() return \"world\" end");

    CuAssertTrue(tc, instance->function_exists("hello") == true);
}



/**
 * Test calling a function we expect to return a string.
 */
void TestStringFunction(CuTest * tc)
{
    /*
     * Get the singleton
     */
    CLua *instance = CLua::instance();
    CuAssertPtrNotNull(tc, instance);

    /*
     * Define the function.
     */
    instance->execute("function hello() return \"world\" end");

    std::string out = instance->function2string("hello", "moi");
    CuAssertTrue(tc, ! out.empty());
    CuAssertStrEquals(tc, out.c_str(), "world");

    /*
     * Redefine the function to return the input argument.
     */
    instance->execute("function hello(name) return \"hello, \" .. name end");

    /*
     * Test on various inputs.
     */
    out = instance->function2string("hello", "moi");
    CuAssertTrue(tc, ! out.empty());
    CuAssertStrEquals(tc, out.c_str(), "hello, moi");

    /*
     * NOTE: Can't test NULL as that won't create a std::string.
     */
    out = instance->function2string("hello", "");
    CuAssertTrue(tc, ! out.empty());
    CuAssertStrEquals(tc, out.c_str(), "hello, ");

    out = instance->function2string("hello", std::to_string(2));
    CuAssertTrue(tc, ! out.empty());
    CuAssertStrEquals(tc, out.c_str(), "hello, 2");

}


CuSuite *
lua_getsuite()
{
    CuSuite *suite = CuSuiteNew();
    SUITE_ADD_TEST(suite, TestLua);
    SUITE_ADD_TEST(suite, TestErrorHandler);
    SUITE_ADD_TEST(suite, TestFunctionToTable);
    SUITE_ADD_TEST(suite, TestFunctionToTableArgs);
    SUITE_ADD_TEST(suite, TestFunctionExists);
    SUITE_ADD_TEST(suite, TestStringFunction);
    return suite;
}
