/*
 * singleton.h - Template base-class implementing the Singleton pattern.
 *
 * This file is part of lumail - http://lumail.org/
 *
 * Copyright (c) 2015 by Steve Kemp.  All rights reserved.
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


#pragma once


/**
 * A template base-class implementing the common Singleton design-pattern.
 *
 * Several of the objects within Lumail are implemented as singletons,
 * including:
 *
 *  - The global input-history.
 *
 *  - The screen interface.
 *
 *  - The embedded Lua intepreter.
 *
 * This template base-class is used to give each of them a common
 * implementation and style.
 *
 */
template <class T> class Singleton
{
public:
    /**
     * Gain access to the singleton-instance.
     */
    static T* instance()
    {
        if (!m_instance)
            m_instance = new T;

        return m_instance;
    };

    /**
     * Destroy the given singleton-instance, if it has been created.
     */
    static void destroy_instance()
    {
      if ( m_instance) {
        delete m_instance;
        m_instance = nullptr;
      }
    };

private:

    /**
     * The one instance of our object.
     */
    static T* m_instance;
};

template <class T> T* Singleton<T>::m_instance = nullptr;
