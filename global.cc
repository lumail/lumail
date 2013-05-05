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
    m_mode = new std::string("maildir");
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
 * Get all selected folders.
 */
std::vector<std::string> CGlobal::get_selected_folders()
{
  return( m_folders );
}

/**
 * Get all folders.
 */
std::vector<CMaildir> CGlobal::get_all_folders(std::string prefix)
{
  std::vector<CMaildir> maildirs;

  std::vector<std::string> folders = CMaildir::getFolders( prefix );
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
  m_folders.clear();
}

  /**
   * Add a folder to the selected set.
   */
void CGlobal::add_folder( std::string path )
{
  m_folders.push_back( path );
}
