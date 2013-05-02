/**
 * global.cc - Singleton interface to store global data
 */


#include <iostream>
#include <cstdlib>
#include <fstream>
#include <string.h>
#include <malloc.h>

#include "global.h"



/**
 * Instance-handle.
 */
CGlobal *CGlobal::pinstance = NULL;



/**
 * Get access to our singleton-object.
 */
CGlobal *CGlobal::Instance()
{
    if (!pinstance)
	pinstance = new CGlobal;

    return pinstance;
}



/**
 * Constructor - This is private as this class is a singleton.
 */
CGlobal::CGlobal()
{
}

