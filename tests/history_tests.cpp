#define CATCH_CONFIG_MAIN

#include "catch.hpp"
#include "history.h"


TEST_CASE( "history/instance", "CHistory singleton tests" )
{
    CHistory *h1 = CHistory::Instance();
    CHistory *h2 = CHistory::Instance();

    REQUIRE( h1 );
    REQUIRE( h2 );

    REQUIRE( h1 == h2 );
}

TEST_CASE( "history/usage", "CHistory basic usage tests" )
{
    /**
     * Get an instance of the history and verify it is empty.
     */
    CHistory *h1 = CHistory::Instance();
    REQUIRE( h1 );
    REQUIRE( h1->size() == 0 );

    /**
     * Add a string.
     */
    std::string one = "This is a test-string\n";
    h1->add( one );

    /**
     * Verify the history is now one-entry long,
     * and that the historys first entry is correct.
     */
    REQUIRE( h1->size() == 1 );
    REQUIRE( h1->at( 0 ) == one );

    /**
     * Clear the history, verify it is back to 0-entries.
     */
    h1->clear();
    REQUIRE( h1->size() == 0 );
}
