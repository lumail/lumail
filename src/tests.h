/*
 * tests.h - Definition of our test-suites.
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


#pragma once

#include "CuTest.h"

/* defined in config_test.cc */
CuSuite *config_getsuite();

/* defined in colour_string_test.cc */
CuSuite *coloured_string_getsuite();

/* defined in directory_test.cc */
CuSuite *directory_getsuite();

/* defined in file_test.cc */
CuSuite *file_getsuite();

/* defined in history_test.cc */
CuSuite *history_getsuite();

/* defined in input_queue_test.cc */
CuSuite *input_queue_getsuite();

/* defined in lua_test.cc */
CuSuite *lua_getsuite();

/* defined in logfile_test.cc */
CuSuite *logfile_getsuite();

/* defined in statuspanel_test.cc */
CuSuite *statuspanel_getsuite();

/* defined in util_test.cc */
CuSuite *util_getsuite();
