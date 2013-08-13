/**
 * global.h - Singleton interface to store global data
 *
 * This file is part of lumail: http://lumail.org/
 *
 * Copyright (c) 2013 by Steve Kemp.  All rights reserved.
 *
 **
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; version 2 dated June, 1991, or (at your
 * option) any later version.
 *
 * On Debian GNU/Linux systems, the complete text of version 2 of the GNU
 * General Public License can be found in `/usr/share/common-licenses/GPL-2'
 */

#ifndef _global_h_
#define _global_h_ 1

#include <unordered_map>
#include <string>
#include <vector>
#include "maildir.h"
#include "message.h"

/**
 * A singleton class to store global data:
 *
 * 1.  The list of currently visible messages.
 *
 * 2.  The list of currently selected folder-names.
 *
 * 3.  The list of currently visible maildirs
 *
 * NOTE:
 *
 *  The alternative to storing the list of selected folder names
 *  is to keep CMaildirs around with "is_selected" attributes, and
 *  I couldn't convince myself that made sense.
 *
 */
class CGlobal
{

public:

    /**
     * Get access to the singleton instance.
     */
    static CGlobal *Instance();

    /**
     * Get all folders which match the current mode: new/all/pattern
     */
    std::vector<CMaildir*> get_folders();

    /**
     * Get all selected folders:
     *  TODO: This should be std::vector<CMaildir *>
     */
    std::vector<std::string> get_selected_folders();

    /**
     * Get all messages from the currently-selected folders.
     */
    std::vector<CMessage *> * get_messages();


    /**
     * Update the global list of messages.
     */
    void update_messages();


    /**
     * Update the global list of Maildirs.
     */
    void update_maildirs();


    /**
     * Remove all selected folders.
     */
    void unset_folders();


    /**
     * Add a folder to the selected set.
     */
    void add_folder(std::string path);


    /**
     * Remove a folder from the selected set.
     */
    bool remove_folder(std::string path);


    /**
     * Get/set the selected folder, i.e. the one with the highlight.
     */
    int get_selected_folder()
    {
        return m_cur_folder;
    }

    /**
     * No wrap-around, because it caused artifacts if you jumped
     * on the last message.
     */
    void set_selected_folder(int offset)
    {
        int count = get_folders().size();
        m_cur_folder = offset;

        if (m_cur_folder >= count)
            m_cur_folder = count-1;
        else if (m_cur_folder < 0)
            m_cur_folder = 0;
    }


    /**
     * Get/Set the selected message.
     */
    int get_selected_message()
    {
        return m_cur_message;
    }
    void set_selected_message(int offset)
    {
        int count = get_messages()->size();

        m_cur_message = offset;

        if (m_cur_message>=count)
            m_cur_message = count-1;
        if (m_cur_message<0)
            m_cur_message=0;
    }


    /**
     * Get/set the message offset.
     */
    int get_message_offset()
    {
        return m_msg_offset;
    }
    void set_message_offset(int offset)
    {
        m_msg_offset = offset;
    }


    /**
     * Get the value of an arbitrary setting.
     */
    std::string * get_variable( std::string name );

    /**
     * Set the value of a variable.
     */
    void set_variable( std::string name, std::string *value );

    /**
     * Get the table of all known settings.
     */
    std::unordered_map<std::string, std::string *> get_variables();

    /**
     * method to invoke the Lua index_hook
     */
    void index_hook();


protected:

    /**
     * Protected functions to allow our singleton implementation.
     */
    CGlobal();
    CGlobal(const CGlobal &);
    CGlobal & operator=(const CGlobal &);

private:

    /**
     * The single instance of this class.
     */
    static CGlobal *pinstance;

    /**
     * The selected folder.
     */
    int m_cur_folder;

    /**
     * The selected message.
     */
    int m_cur_message;

    /**
     * The line-number of the message to start drawing at.
     */
    int m_msg_offset;


    /**
     * Currently selected folders.
     */
    std::vector < std::string > m_selected_folders;

    /**
     * The list of currently visible messages.
     */
    std::vector<CMessage*> *m_messages;

    /**
     * The list of all currently visible maildirs.
     */
    std::vector<CMaildir *> *m_maildirs;

    /**
     * The settings we hold.
     */
    std::unordered_map<std::string, std::string *> m_variables;

};

#endif /* _global_h_ */
