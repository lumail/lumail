/**
 * Stub code for outputing the body from a message.
 */

#include <stdint.h>
#include <iostream>
#include <iostream>
#include <fstream>
#include <sstream>
#include <string>

#include <mimetic/mimetic.h>


int main( int argc, char *argv[] )
{
  if ( argc != 2 ) {
    std::cout << "Usage " << argv[0] << " message-file" << std::endl;
    return 1;
  }

  /**
   * Get the entity.
   */
  std::ifstream file(argv[1]);
  mimetic::MimeEntity *me = new mimetic::MimeEntity(file);

  /**
   * Did we find the first text/plain part?
   */
  bool found = false;

  mimetic::Header & h = me->header();
  std::cout << "File   : " << argv[1] << std::endl;
  std::cout << "Subject: " << (h.subject()) << std::endl;
  std::cout << "From: " << (h.from()) << std::endl;

  /**
   * Iterate over every part.
   */
  mimetic::MimeEntityList& parts = me->body().parts();
  mimetic::MimeEntityList::iterator mbit = parts.begin(), meit = parts.end();
  for(; mbit != meit; ++mbit) {

    /**
     * Get the content-type.
     */
    std::string type = (*mbit)->header().contentType().str();

    /**
     * If we've found text/plain print it out.
     */
    if ( type.find( "text/plain" ) != std::string::npos )
      {
        if ( !found ) {
          std::cout << std::endl;
          std::cout << std::endl;
          std::cout << (*mbit)->body() << std::endl;
          found = true;
        }
      }
    else
      {
        if ( ! found )
          std::cout  << "Ignoring content-type: " << type << std::endl;
      }
  }

  /**
   * Cleanup.
   */
  delete(me);
  std::cout << "All done." << std::endl;
  return 0;
}
