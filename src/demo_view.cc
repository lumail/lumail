#include <cursesw.h>


#include "demo_view.h"


/**
 * Constructor.  NOP.
 */
CDemoView::CDemoView ()
{
}

/**
 * Destructor.  NOP.
 */
CDemoView::~CDemoView ()
{
}

/**
 * This is the virtual function which is called to refresh the display
 * when the global.mode == "demo"
 */
void
CDemoView::draw ()
{
    mvprintw (10, 10, "Hello World - This is 'demo' mode");
}
