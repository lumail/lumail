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
