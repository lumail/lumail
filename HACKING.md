
Overview
--------

`CScreen` implements the main user-interface, and runs the event loop:

* Creates each of the view-modes, in a map.
* Polling for key-presses.
    * When they timeout, redraw the screen.

`CGlobalState` is responsible for maintaining the current list of Maildirs,
the current list of Messages, and getting/setting the current message.

The different modal-modes are implemented in `src/*_view.cc`, and each
is instantiated in the `CScreen` setup phase.  Views each have a name,
and routines for drawing the display.

Configuration values are handled via `CConfig`, and when changes are
made these are broadcast to:

* The `CGlobalState` object, such that state-changes can happen.
* The Lua function `Config:key_changed` if it is defined, so user-actions can occur.



Wrapping C++ to Lua
-------------------

The rule of thumb is that when we want to wrap a C++ object
we create a new file with a `_lua.cc` suffix.

So for example our configuration getter/setter class is `CConfig`,
which is implemented in `config.[cc h]`.  The Lua wrapper to this
class is defined in `config_lua.cc`.

The wrappers are very simple, and follow a template.


(Display) Modes
---------------

Lumail is a modal editor, and the modes are implemented in :

    src/$mode_view.{cc h}

Modes are looked up by name, from a hash-map, and are derived from
the `CModeView` class. To write a new mode:

* Define a class inheriting from `CModeView`
* Implement `draw()` to draw it
* Implement `on_idle()` if you need timed-events to happen.

The `draw` method may draw to the screen with standard curses functions,
or it might send a vector of text-lines to `CScreen::draw_text_lines`.

Using the `draw_text_lines` method ensure there is little duplication
between modes.


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
