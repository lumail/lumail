/*
 * attachment_view.h - Draw a list of attachments
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



/**
 * Only include this header one time.
 */
#pragma once

#include "screen.h"


/**
 * This is a attachment-view of the screen - it shows lists of attachments.
 *
 */
class CAttachmentView: public CViewMode
{

public:
    /**
     * Constructor / Destructor.
     */
    CAttachmentView();
    ~CAttachmentView();

    /**
     * Drawing routine - called when the current.mode=="message".
     */
    void draw();

    /**
     * Called when things are idle.
     */
    void on_idle();
private:

    /**
     * Get the output of calling `attachment_view`, which is the text we'll display.
     */
    std::vector<std::string> get_text();

};
