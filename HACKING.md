
Overview
--------

The core of the code is the main-loop implemented in `screen.cc`.


(Display) Modes
---------------

Lumail is a modal editor, and the modes are implemented in :

    src/$mode_view.{cc h}

Modes are looked up by name, from a hash-map, and are derived from
the `CModeView` class. To write a new mode:

* Define a class inheriting from `CModeView`
* Implement `draw()` to draw it
* Implement `on_idle()` as appropriate.

The `draw` method may draw to the screen with standard curses functions,
or it might send a vector of text-lines to `CScreen::draw_text_lines`.  This
ensures that there is no duplication in selection/navigation


Navigation
----------

To avoid having multiple functions of the form:

* maildir_next
* message_next
* index_next

We define all offset-related things as variables.  For example the
maildir-mode, which shows a list of maildirs will get/set the following
variables:

* `maildir.current` - The current offset of the selected item
* `maildir.max` - The number of possible maildirs

This is a little odd, perhaps, but makes sense to me.
