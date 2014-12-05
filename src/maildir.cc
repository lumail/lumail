
#include <algorithm>
#include <cstdlib>
#include <dirent.h>
#include <fcntl.h>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string.h>
#include <string>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <unordered_map>
#include <vector>


#include <gmime/gmime.h>


#include "maildir.h"
#include "message.h"



CMaildir::CMaildir(const std::string name)
{
    m_path = name;
}


bool CMaildir::is_directory(std::string path)
{
    struct stat sb;

    if (stat(path.c_str(), &sb) < 0)
        return false;

    return (S_ISDIR(sb.st_mode));
}


bool CMaildir::is_maildir()
{
    std::vector < std::string > dirs;
    dirs.push_back(m_path);
    dirs.push_back(m_path + "/cur");
    dirs.push_back(m_path + "/tmp");
    dirs.push_back(m_path + "/new");

    for (std::vector < std::string >::iterator it = dirs.begin(); it != dirs.end(); ++it)
    {
        if (!CMaildir::is_directory(*it))
            return false;
    }
    return true;
}


/**
 * This method is bogus.  Ideally we'd cache C++ objects
 * on the mtime of the directory and return a vector of CMessage
 * objects.
 *
 * Instead we're returning a vector of paths.
 */
std::vector < std::string > CMaildir::messages()
{
    std::vector < std::string > tmp;

    dirent *de;
    DIR *dp;

    /**
     * Directories we search.
     */
    std::vector < std::string > dirs;
    dirs.push_back(m_path + "/cur/");
    dirs.push_back(m_path + "/new/");

    /**
     * For each directory.
     */
    for (std::vector < std::string >::iterator it = dirs.begin(); it != dirs.end(); ++it)
    {
        std::string path = *it;
        dp = opendir(path.c_str());
        if (dp)
        {
            while (true)
            {
                de = readdir(dp);
                if (de == NULL)
                    break;

                /** Maybe we should check for DT_REG || DT_LNK ? */
                if ((de->d_type != DT_DIR)
                    || (de->d_type == DT_UNKNOWN
                        && !CMaildir::is_directory(std::string(path + de->d_name))))
                {

                    if (de->d_name[0] != '.')
                    {
                        tmp.push_back(path + de->d_name);
                    }
                }
            }
            closedir(dp);
        }
    }
    return tmp;
}

std::string CMaildir::path()
{
    return (m_path);
}


CMaildir::~CMaildir()
{
}


/**
 * The number of new messages for this maildir.
 */
int CMaildir::unread_messages()
{
    update_cache();
    return( m_unread );
}


/**
 * The total number of messages for this maildir.
 */
int CMaildir::total_messages()
{
    update_cache();
    return( m_total );
}



/**
 * Update the cached total/unread message counts.
 */
void CMaildir::update_cache()
{
    /**
     * If the cached date isn't different then we need do nothing.
     */
    time_t last_mod = last_modified();

    /**
     * If we've got -1 for the count/unread then we've
     * just been created.
     *
     */
    if ( ( last_mod <= m_modified ) &&
         ( m_unread != -1 ) &&
         ( m_total != -1 ) )
        return;

    m_modified = last_mod;


    // /**
    //  * Get all messages, and update the total
    //  */
    // CMessageList all = getMessages();
    // m_total = all.size();


    // /**
    //  * Now update the unread count.
    //  */
    // m_unread = 0;
    // for (std::shared_ptr<CMessage> message : all)
    // {
    //     if ( message->is_new() )
    //         m_unread++;
    // }
}

/**
 * Return the last modified time for this Maildir.
 */
time_t CMaildir::last_modified()
{
    time_t last = 0;
    struct stat st_buf;

    std::string p = path();

    /**
     * The two directories we care about: new/ + cur/
     */
    std::vector<std::string> dirs;
    dirs.push_back(p + "/cur");
    dirs.push_back(p + "/new");

    /**
     * See which was the most recently modified.
     */
    for (std::string dir : dirs)
    {
        /**
         * If we can stat() the dir and it is more recent
         * than the current value - update it.
         */
        if ( ! stat(dir.c_str(),&st_buf) )
            if ( st_buf.st_mtime > last )
                last = st_buf.st_mtime;
    }

    return( last );
}
