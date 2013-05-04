/**
 * maildir.cc - Utility functions for working with Maildirs
 */


#include <vector>
#include <algorithm>
#include <sys/stat.h>
#include <sys/types.h>
#include <dirent.h>
#include <sys/types.h>
#include "maildir.h"
#include "message.h"


/**
 * Constructor.  NOP
 */
CMaildir::CMaildir( std::string path )
{
  m_path = path;
}

/**
 * Destructor.  NOP.
 */
CMaildir::~CMaildir()
{
}


/**
 * The number of new messages for this directory.
 */
int CMaildir::newMessages()
{
  return( CMaildir::countFiles( m_path + "/new" ) );
}


/**
 * The number of read messages for this directory.
 */
int CMaildir::availableMessages()
{
  return( CMaildir::countFiles( m_path + "/cur" ) );
}


/**
 * The friendly name of the maildir.
 */
std::string CMaildir::name()
{
  unsigned found = m_path.find_last_of("/");
  return( m_path.substr(found+1) );
}


/**
 * Count files in a directory.
 */
int CMaildir::countFiles( std::string path )
{
  int count = 0;
  dirent* de;
  DIR* dp;

  dp = opendir( path.c_str() );
  if (dp)
    {
    while (true)
      {
        de = readdir( dp );
        if (de == NULL)
          break;

        if ( !CMaildir::isDirectory( std::string( path + "/" + de->d_name ) ) )
          count += 1;
      }
    closedir( dp );
    }
  return count;
}


/**
 * Return a sorted list of maildirs beneath the given path.
 */
std::vector<std::string> CMaildir::getFolders( std::string path )
{
  std::vector <std::string> result;
  dirent* de;
  DIR* dp;

  std::string prefix = path.empty() ? "." : path.c_str() ;
  dp = opendir( prefix.c_str() );
  if (dp)
    {
    while (true)
      {
        de = readdir( dp );
        if (de == NULL)
          break;

        if ( CMaildir::isMaildir( std::string( prefix + "/" + de->d_name ) ) )
          result.push_back( std::string( prefix + "/" + de->d_name ) );
      }
    closedir( dp );
    std::sort( result.begin(), result.end() );
    }
  return result;
}



/**
 * Get the messages in the folder.
 */
std::vector<CMessage> CMaildir::getMessages()
{
  std::vector <CMessage> result;
  dirent* de;
  DIR* dp;

  dp = opendir( ( m_path + "/cur" ).c_str() );
  if (dp)
    {
    while (true)
      {
        de = readdir( dp );
        if (de == NULL)
          break;

        if ( !CMaildir::isDirectory( std::string( m_path + "/cur/" + de->d_name ) ) )
          result.push_back( CMessage(std::string( m_path + "/cur/" + de->d_name ) ) );
      }
    closedir( dp );
    }

  dp = opendir( ( m_path + "/new" ).c_str() );
  if (dp)
    {
    while (true)
      {
        de = readdir( dp );
        if (de == NULL)
          break;

        if ( !CMaildir::isDirectory( std::string( m_path + "/new/" + de->d_name ) ) )
          result.push_back( CMessage(std::string( m_path + "/new/" + de->d_name ) ) );
      }
    closedir( dp );
    }
  return result;
}

/**
 * Is the given path a Maildir?
 */
bool CMaildir::isMaildir(std::string path)
{
    std::vector < std::string > dirs;
    dirs.push_back(path);
    dirs.push_back(path + "/cur");
    dirs.push_back(path + "/tmp");
    dirs.push_back(path + "/new");

    std::vector < std::string >::iterator it;
    for (it = dirs.begin(); it != dirs.end(); ++it) {
	if (!CMaildir::isDirectory(*it))
	    return false;
    }
    return true;
}


/**
 * Is the given path a directory?
 */
bool CMaildir::isDirectory(std::string path)
{
    struct stat sb;

    if (stat(path.c_str(), &sb) < 0)
	return 0;

    return (S_ISDIR(sb.st_mode));

}

