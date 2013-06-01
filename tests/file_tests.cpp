#define CATCH_CONFIG_MAIN

#include "catch.hpp"
#include "file.h"
#include <sys/stat.h>


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



/**
 * Generate two random files.
 *
 * Delete the second.
 *
 * Move the first one to the second.
 *
 * Ensure the first is gone, and that the second has the
 * correct contents/
 */
TEST_CASE( "file/move", "CFile::move tests" )
{
    /**
     * Generate a temporary file for our move-source.
     * Generate a second temporary file for oure move-destination.
     */
    char file1[] = "/tmp/file.test.src.XXXXXX";
    int fd1       = mkstemp(file1);

    char file2[] = "/tmp/file.test.dst.XXXXXX";
    int fd2       = mkstemp(file2);

    REQUIRE( fd1 != -1 );
    REQUIRE( fd2 != -1 );
    close(fd1);
    close(fd2);

    /**
     * Delete the second file, and ensure it is gone.
     */
    unlink( file2 );
    REQUIRE( CFile::exists( file2 ) == false );

    /**
     * Open the first file to write known-good content to it.
     */
    FILE *f = fopen( file1, "w" );
    fprintf(f, "This is a test\n" );
    fclose( f );

    /**
     * Get the size of the first file.  Esnure it matches.
     */
    struct stat st;
    REQUIRE( stat(file1, &st) == 0 );
    REQUIRE( st.st_size == 15 );

    /**
     * Move the file now.
     */
    CFile::move( file1, file2 );

    /**
     * So the source should not exist, but the destination should.
     */
    REQUIRE( CFile::exists( file1 ) == false );
    REQUIRE( CFile::exists( file2 ) == true );

    /**
     * The destination should also have the correct size.
     */
    REQUIRE( stat(file2, &st) == 0 );
    REQUIRE( st.st_size == 15 );

    /**
     * Now cleanup both files.
     */
    unlink( file1 );
    REQUIRE( CFile::exists( file1 ) == false );
    unlink( file2 );
    REQUIRE( CFile::exists( file2 ) == false );
}
