/**
 * message.cc - A class for working with a single message.
 */

#include <stdint.h>
#include <iostream>
#include <iostream>
#include <fstream>
#include <sstream>
#include <string>

#include <mimetic/mimetic.h>

#include "message.h"
#include "global.h"

using namespace std;
using namespace mimetic;


/**
 * Constructor.
 */
CMessage::CMessage(std::string filename)
{
    m_path = filename;
}


/**
 * Destructor.
 */
CMessage::~CMessage()
{
}


/**
 * Get the path to the message.
 */
std::string CMessage::path()
{
    return (m_path);
}


/**
 * Get the flags for this message.
 */
std::string CMessage::flags()
{
    std::string flags = "";

    if (m_path.empty())
	return (flags);

    unsigned offset = m_path.find(":2,");
    if (offset != std::string::npos)
      flags = m_path.substr(offset + 3);

    if ( flags.size() > 3)
      flags = "";

    /**
     * Sleazy Hack.
     */
    if ( m_path.find( "/new/" ) != std::string::npos )
      flags += "N";

    return flags;
}


/**
 * Format the message for display in the header - via the lua format string.
 */
std::string CMessage::format()
{
  /**
   * Read the message.
   */
  ifstream file(path().c_str());
  MimeEntity me(file);

  /**
   * Get the header.
   */
  Header & h = me.header();

  /**
   * Get the format-string we'll expand.
   */
  CGlobal *global = CGlobal::Instance();
  std::string *fmt = global->get_index_format();
  std::string result = std::string(*fmt);


  /**
   * The variables we know about.
   */
  const char *fields[6] = { "FLAGS", "FROM", "TO", "SUBJECT",  "DATE", 0 };
  const char **std_name = fields;


  /**
   * Iterate over everything we could possibly-expand.
   */
  for( int i = 0 ; std_name[i] ; ++i) {

    unsigned int offset = result.find( std_name[i], 0 );

    if ( ( offset != std::string::npos ) && ( offset < result.size() ) ) {

      /**
       * The bit before the variable, the bit after, and the body we'll replace
       * the key with.
       */
      std::string before = result.substr(0,offset-1);
      std::string body = "";
      std::string after  = result.substr(offset+strlen(std_name[i]));

      /**
       * Stub-bodies for the variables.
       */
      if ( strcmp(std_name[i] , "TO" ) == 0 ) {
        body = h.to().str();
      }
      if ( strcmp(std_name[i] , "DATE" ) == 0 ) {
        if (h.hasField("date"))
          body = h.field("date").value();
      }
      if ( strcmp(std_name[i] , "FROM" ) == 0 ) {
        body += h.from().str();
      }
      if ( strcmp(std_name[i] , "FLAGS" ) == 0 ) {
        std::string flg = flags();
        while( (int)strlen(flg.c_str()) < 3 )
          flg = std::string(" ") + flg;

        body = flg;
      }
      if ( strcmp(std_name[i] , "SUBJECT" ) == 0 ) {
        body = h.subject();
      }

      result = before + body + after;
    }
  }



  return( result );
}
