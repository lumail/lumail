#define CATCH_CONFIG_MAIN

#include "catch.hpp"
#include "file.h"


TEST_CASE( "file/exists", "CFile::exists tests" )
{
    /**
     * These files should be present.
     */
    REQUIRE( CFile::exists( "/etc/passwd" ) );
    REQUIRE( CFile::exists( "/etc/../etc/fstab" ) );

    /**
     * These are non-existant files.
     */
    REQUIRE_FALSE( CFile::exists( "/not/a/real.file" ) );
    REQUIRE_FALSE( CFile::exists( "/I/like/cake" ) );
}


TEST_CASE( "file/is_directory", "CFile::is_directory tests" )
{
    /**
     * These should be present.
     */
    REQUIRE( CFile::is_directory( "/etc/" ) );
    REQUIRE( CFile::is_directory( "/etc/../etc/" ) );

    /**
     * These entries are not directories.
     */
    REQUIRE_FALSE( CFile::is_directory( "/etc/motd" ) );
    REQUIRE_FALSE( CFile::is_directory( "/etc/passwd" ) );
}


TEST_CASE( "file/copy", "CFile::copy tests" )
{
    /**
     * Generate a temporary file.  Test that it doesn't exist.
     * then copy a known-good file there, and ensure it does.
     */
    char filename[] = "/tmp/file.test.XXXXXX";
    int fd          = mkstemp(filename);

    REQUIRE( fd != -1 );

    /**
     * OK the file should now exist.
     */
    REQUIRE( CFile::exists( filename ) );

    /**
     * Now delete it, and ensure it is gone.
     */
    unlink( filename );
    REQUIRE_FALSE( CFile::exists( filename ) );

    /**
     * Copy the real file there.
     */
    CFile::copy( "/etc/fstab", filename );
    REQUIRE( CFile::exists( filename ) );

    /**
     * Cleanup and ensure we're clean.
     */
    unlink( filename );
    REQUIRE_FALSE( CFile::exists( filename ) );

}
