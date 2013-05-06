/**
 * global.cc - Singleton interface to store global data
 */


#include <iostream>
#include <cstdlib>
#include <fstream>
#include <string.h>
#include <malloc.h>

#include "global.h"



/**
 * Instance-handle.
 */
CGlobal *CGlobal::pinstance = NULL;



/**
 * Get access to our singleton-object.
 */
CGlobal *CGlobal::Instance()
{
    if (!pinstance)
	pinstance = new CGlobal;

    return pinstance;
}


/**
 * Constructor - This is private as this class is a singleton.
 */
CGlobal::CGlobal()
{
  /**
   * Defaults.
   */
    m_mode           = new std::string("maildir");
    m_maildir_limit  = new std::string("all");
    m_maildir_prefix = NULL;
    m_cur_folder     = 0;
    m_cur_message    = 0;
}


/**
 * Set the new mode for the client.
 */
void CGlobal::set_mode(std::string * mode)
{
    if (m_mode != NULL)
	delete(m_mode);

    m_mode = new std::string(mode->c_str());
}


/**
 * Get the current mode the client is in: index, maildir or message.
 */
std::string * CGlobal::get_mode()
{
    return (m_mode);
}


/**
 * Set the maildir limit.
 */
void CGlobal::set_maildir_limit( std::string *limit )
{
  if ( m_maildir_limit )
    delete(m_maildir_limit );

  m_maildir_limit = limit;
}


/**
 * Get the maildir limit
 */
std::string * CGlobal::get_maildir_limit()
{
  return( m_maildir_limit );
}


/**
 * Set the prefix for our maildir folders.
 */
void CGlobal::set_maildir_prefix( std::string *prefix )
{
  if ( m_maildir_prefix != NULL )
    delete( m_maildir_prefix );

  m_maildir_prefix = new std::string(prefix->c_str());
}


/**
 * Get the prefix for the maildir folders.
 */
std::string * CGlobal::get_maildir_prefix()
{
  return( m_maildir_prefix );
}


/**
 * Get all selected folders.
 */
std::vector<std::string> CGlobal::get_selected_folders()
{
  return( m_selected_folders );
}


/**
 * Get all folders.
 */
std::vector<CMaildir> CGlobal::get_all_folders()
{
  std::vector<CMaildir> maildirs;

  std::vector<std::string> folders = CMaildir::getFolders( *m_maildir_prefix );
  std::vector < std::string >::iterator it;
  for (it = folders.begin(); it != folders.end(); ++it) {
    maildirs.push_back( CMaildir( *it ) );
  }

  return( maildirs );
}

/**
 * Get folders matching the current mode.
 */
std::vector<CMaildir> CGlobal::get_folders()
{
  CGlobal               *global = CGlobal::Instance();
  std::vector<CMaildir> folders = global->get_all_folders();
  std::string           *filter = global->get_maildir_limit();
  std::vector<CMaildir> display;

  /**
   * Filter the folders to those we can display
   */
  std::vector<CMaildir>::iterator it;
  for (it = folders.begin(); it != folders.end(); ++it)
    {
      CMaildir x = *it;

      if ( strcmp( filter->c_str(), "all") == 0 ){
        display.push_back( x );
      }
      else if ( strcmp( filter->c_str(), "new") == 0 )  {
        if ( x.newMessages() > 0 ) {
          display.push_back( x );
        }
      }
      else {
        std::string  path = x.path();
        if ( path.find( *filter, 0 ) !=std::string::npos ) {
            display.push_back( x );
        }
      }
    }

  return( display );
}


/**
 * Remove all selected folders.
 */
void CGlobal::unset_folders()
{
  m_selected_folders.clear();
}


/**
 * Add a folder to the selected set.
 */
void CGlobal::add_folder( std::string path )
{
  m_selected_folders.push_back( path );
}
