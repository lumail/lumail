/**
 * Stub code for testing we can expand variables in a std::string.
 *
 */

#include <cstdlib>
#include <string>
#include <string.h>
#include <iostream>
#include <fstream>

int main( int argc, char *argv[] )
{
  /**
   * The format string we're starting with.
   */
  std::string format = "[$FLAGS] - $FROM - $SUBJECT";
  std::cout << "Expanding format string: " << format << std::endl;

  /**
   * The variables we know about.
   */
  const char *fields[8] = { "FLAGS", "FROM", "TO", "SUBJECT", "CC", "BCC", "DATE", 0 };
  const char **std_name = fields;

  /**
   * Iterate over everything we could possibly-expand.
   */
  for( int i = 0 ; std_name[i] ; ++i)
    {
      unsigned int offset = format.find( std_name[i], 0 );
      if ( ( offset != std::string::npos ) && ( offset < format.size() ) )
        {
          // std::cout << "Found " << std_name[i] << " at offset " << offset << std::endl;

          /**
           * The bit before the variable, the bit after, and the body we'll replace
           * the key with.
           */
          std::string before = format.substr(0,offset-1);
          std::string body = "";
          std::string after  = format.substr(offset+strlen(std_name[i]));

          /**
           * Stub-bodies for the variables.
           */
          if ( strcmp(std_name[i] , "TO" ) == 0 )
            body = "steve@steve.org.uk";
          if ( strcmp(std_name[i] , "FROM" ) == 0 )
            body = "steve@steve.org.uk";
          if ( strcmp(std_name[i] , "FLAGS" ) == 0 )
            body = "NRS";
          if ( strcmp(std_name[i] , "SUBJECT" ) == 0 )
            body = "MY LOVELY SUBJECT";

          format = before + body + after;
        }
    }

  std::cout << "End result: " << format << std::endl;
  return 0;
}
