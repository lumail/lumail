#include <string>
#include <string.h>
#include <fcntl.h>
#include <iostream>
#include <fstream>
#include <stdint.h>
#include <cassert>


/**
 * Expand a string such as '$SUBJECT{min:10 max:20 color:red} - Moi"
 * into something suitable.
 */
std::string expand(std::string input , std::string subject)
{
    /**
     * Split into the token and the optional arguments.
     */


    /**
     * Convert "$SUBJECT" to the specified subject.
     */

    /**
     * If min > 0 .. pad if necessary.
     */

    /**
     * If max > 0 truncate.
     */

    return( input );
}



/**
 * Simple test.
 */
int main( int argc, char *argv[] )
{

    assert( expand( "$SUBJECT{min:10}", "steve" ) == "steve     " );
    assert( expand( "$SUBJECT{max:3}", "12345" ) == "123" );

    return 0;
}
