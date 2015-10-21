

#ifndef _CFILE_H
#define _CFILE_H 1


#include <vector>
#include <string>


/**
 * A collection of file utility primitives.
 */
class CFile
{

  public:

    /**
     * Does a file exist?
     */
    static bool exists (std::string path);


    /**
     * Is the given file executable?
     */
    static bool executable (std::string path);


    /**
     * Is the given path a directory?
     */
    static bool is_directory (std::string path);


    /**
     * Get the files in the given directory.
     *
     * NOTE: Directories are excluded.
     */
    static std::vector < std::string > files_in_directory (std::string path);


    /**
     * Delete a file.
     */
    static bool delete_file (std::string path);


    /**
     * Get the basename of a file.
     */
    static std::string basename (std::string path);


    /**
     * Copy a file.
     */
    static void copy (std::string src, std::string dest);


    /**
     * Edit a file, using the users preferred editor.
     */
    static int edit (std::string filename);


    /**
     * Move a file.
     */
    static bool move (std::string src, std::string dest);


    /**
     * Send the contents of a file to the given command, via popen.
     */
    static bool file_to_pipe (std::string src, std::string cmd);


    /**
     * Return a sorted list of maildirs beneath the given prefix.
     */
    static std::vector < std::string > get_all_maildirs (std::string prefix);

    /**
     * Allow completion of file/path-names
     */
    static std::vector < std::string > complete_filename (std::string path);

};

#endif /*  _CFILE_H  */
