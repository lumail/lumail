#pragma once

#include <algorithm>


/**
 * A filter which is used by std::erase to remove duplicate slash-characters.
 */
struct both_slashes
{
    /**
     * This implements the filter process, returning true if both
     * characters "a" and "b" are "/".
     */
    bool operator()(char a, char b) const
    {
        return a == '/' && b == '/';
    }
};
