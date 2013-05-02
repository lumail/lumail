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
  /**
   * Defaults.
   */
    m_mode = new std::string("maildir");
}


void CGlobal::set_mode(std::string * mode)
{
    if (m_mode != NULL)
	delete(m_mode);

    m_mode = new std::string(mode->c_str());
}

std::string * CGlobal::get_mode()
{
    return (m_mode);
}
