#ifndef _MESSAGE_H
#define _MESSAGE_H 1

#include <unordered_map>
#include <string>


/**
 * This is the C++ object which represents an email message.
 *
 * The lua binding is modeled after this class structure too.
 *
 */
class CMessage
{
public:
    /**
     * Constructor.
     */
    CMessage(const std::string name);

    /**
     * Destructor.
     */
    ~CMessage();

    /**
     * Get the path of this message.
     */
    std::string path();

    /**
     * Get the value of the given header.
     */
    std::string header(std::string name);

    /**
     * Get all headers, and their values.
     */
    std::unordered_map < std::string, std::string > headers();

    /**
     * Retrieve the current flags for this message.
     */
    std::string get_flags();

    /**
     * Set the flags for this message.
     */
    void set_flags( std::string new_flags );

    /**
     * Add a flag to a message.
     */
    bool add_flag( char c );

    /**
     * Does this message possess the given flag?
     */
    bool has_flag( char c );

    /**
     * Remove a flag from a message.
     */
    bool remove_flag( char c );



  private:

    /**
     * The path on-disk to the message.
     */
     std::string m_path;
};



#endif                          /* _MESSAGE_H  */
