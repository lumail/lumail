/*
 * config_test.cc - Test-cases for our CConfig class.
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

#include "config.h"
#include "CuTest.h"



/**
 * Test we have no keys by default.
 *
 * This might require explanation : We are looking for an empty
 * set of keys, but some values are created at constructor-time,
 * such as the version of Lua, the version of Lumail.
 *
 * Because of that we're looking for a non-zero set of keys even though
 * we pretend we're looking for zero.
 */
void TestEmptyConfig(CuTest * tc)
{
    CConfig *instance             = CConfig::instance();
    std::vector<std::string> keys = instance->keys();

    CuAssertIntEquals(tc, keys.size(), 8);
}


/**
 * Test that config names are unique.
 */
void TestKeynames(CuTest * tc)
{
    CConfig *config = CConfig::instance();

    /**
     * Count the original set of keys
     */
    std::vector<std::string> orig = config->keys();

    /**
     * Add a key=value
     */
    config->set("steve", "kemp", false);

    /**
     * The new key is a string.
     */
    CConfigEntry *val = config->get("steve");
    CuAssertTrue(tc, val->type == CONFIG_STRING);


    /**
     * We now have one more key.
     */
    std::vector<std::string> updated1 = config->keys();
    CuAssertTrue(tc, updated1.size() == (orig.size() + 1));


    /**
     * Now set the value to be an integer.
     */
    config->set("steve", 1, false);

    /**
     * The key now has an integer value.
     */
    val = config->get("steve");
    CuAssertTrue(tc, val->type == CONFIG_INTEGER);

    /**
     * The count of keys is still the same - the new value replaced the old.
     */
    std::vector<std::string> updated2 = config->keys();
    CuAssertTrue(tc, updated2.size() == (orig.size() + 1));
    CuAssertTrue(tc, (updated1.size() == updated2.size()));

}


/**
 * Test that we can delete all our known key/values.
 */
void TestKeyDeletion(CuTest * tc)
{
    CConfig *config = CConfig::instance();

    config->set("steve", "kemp");
    config->set("lumi", "kemp");
    config->set("pi", 3);
    config->set("pie", "delicious");

    /*
     * Count keys
     */
    std::vector<std::string> keys = config->keys();
    CuAssertTrue(tc, (keys.size() > 3));


    /*
     * Fetching one works
     */
    CConfigEntry *value = config->get("pie");
    CuAssertPtrNotNull(tc, value);

    /*
     * Empty the keys
     */
    config->remove_all();

    /*
     * Now we've empted the count should be zero
     */
    keys = config->keys();
    CuAssertIntEquals(tc, 0, keys.size());

    /*
     * And fetching should fail.
     */
    value = config->get("pie");
    CuAssertPtrEquals(tc, NULL, value);
}


CuSuite *
config_getsuite()
{
    CuSuite *suite = CuSuiteNew();
    SUITE_ADD_TEST(suite, TestEmptyConfig);
    SUITE_ADD_TEST(suite, TestKeynames);
    SUITE_ADD_TEST(suite, TestKeyDeletion);
    return suite;
}
