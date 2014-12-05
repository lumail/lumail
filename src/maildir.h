

#ifndef _MAILDIR_H
#define _MAILDIR_H 1

#include <string>
#include <vector>



/**
 * Maildir object.
 *
 * This is the C++ implementation of the maildir class.
 *
 */
class CMaildir {
public:

    /**
     * COnstructor.
     */
    CMaildir(const std::string name);


    /**
     * Destructor
     */
    ~CMaildir();


    /**
     * Is the given path a directory?
     */
    static bool is_directory(std::string path);


    /**
     * Does the given path contain a maildir ?
     */
    bool is_maildir();

    /**
     * This method is bogus.  Ideally we'd cache C++ objects
     * on the mtime of the directory and return a vector of CMessage
     * objects.
     *
     * Instead we're returning a vector of paths.
     */
     std::vector < std::string > messages();

     /**
      * Return the path.
      */
     std::string path();

     /**
      * The number of new messages for this maildir.
      */
     int unread_messages();

     /**
      * The total number of messages for this maildir.
      */
     int total_messages();


private:
     std::string m_path;

    /**
     * Cached time/date object.
     */
    time_t m_modified;

    /**
     * Cached unread-count + cached total count.
     */
    int m_unread;
    int m_total;

    /**
     * Return the last modified time for this Maildir.
     * Used to determine if we need to update our cache.
     */
    time_t last_modified();

    /**
     * Update the cached total/unread message counts.
     */
    void update_cache();

};



#endif                          /* _MAILDIR_H */
