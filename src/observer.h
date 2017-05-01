/*
 * observer.h - Implementation of the Observer-pattern.
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

#include <vector>
#include <string>


/*
 * Forward-declaration.
 */
class CConfigEntry;


/**
 * This is the subject-class for the observer-pattern.
 *
 * This class-interface allows watchers to register themselves
 * via the `attach` method - and then they will be updated
 * whenever something changes.
 */
class Subject
{
public:
    /**
     * This vector contains the registered observers of this
     * subject.
     *
     */
    std::vector < class Observer * > views;

public:
    /**
     * Attach a new observer to this subject.
     */
    void attach(Observer *obs)
    {
        views.push_back(obs);
    };
};



/**
 * This interface allows a class to be notified of a change
 * in state, via our subject-class.
 *
 * This is part of the Observer-pattern, but it is a simplified
 * implementation because we don't keep a reference to the
 * actual subject - instead when we broadcast the "update"
 * message we transmit the change as parameter.
 *
 * We do this because in Lumail the only observable is the
 * CConfig class, and we only need to issue updates on the
 * key-names which have changed.
 *
 */
class Observer
{
public:

    /**
     * Constructor.
     *
     * Call this with a reference to the subject you wish to be watching.
     *
     * When a change is made then the update-method will be called later.
     */
    Observer(Subject *mod)
    {
        mod->attach(this);
    }

    /**
     * This is the virtual function sub-classes much implement to
     * be notified of key-changes.
     */
    virtual void update(std::string name, CConfigEntry *old) = 0;

};
