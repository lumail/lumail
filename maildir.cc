/**
 * maildir.cc - Utility functions for working with Maildirs
 */


#include <vector>
#include <sys/stat.h>
#include <sys/types.h>
#include "maildir.h"



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
