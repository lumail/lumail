#pragma once

#include "CuTest.h"

/* defined in config_test.cc */
CuSuite *config_getsuite();

/* defined in coloured_string_test.cc */
CuSuite *coloured_string_getsuite();

/* defined in file_test.cc */
CuSuite *file_getsuite();

/* defined in util_test.cc */
CuSuite *util_getsuite();
