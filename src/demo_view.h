#ifndef _DEMO_VIEW_H
#define _DEMO_VIEW_H 1

#include "screen.h"


/**
 * This is a demo-view of the screen.
 */
class CDemoView:public CViewMode
{

  public:
  /**
   * Constructor / Destructor.
   */
    CDemoView ();
    ~CDemoView ();

  /**
   * Drawing routine - called when the current.mode=="demo".
   */
    void draw ();
};

#endif
