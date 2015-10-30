API
===

The core idea of lumail2, and the reason for writing it rather than continuing to extend
the version one project, is that everything is an object.

At the moment we have objects for working with:

* Configuration variables.
* Files.
* Maildirs.
* Messages.
* The screen.
    * The status-panel, which is optionally displayed upon the screen.


Variables
---------

We have a number of variables which are special, the most important ones are:

* `maildir.prefix`
    * This holds the prefix to the maildir hierarchy.
* `global.editor`
    * The user's editor.
* `global.from`
    * The email address to send messages from.
* `global.history`
    * The name of the file to write input-history to.
* `global.horizontal`
    * The horizontal-offset used to implement left/right scrolling.
* `global.mode`
    * This holds the name of the currently active display-mode

For each mode that has a display there will be a `$mode.max` to store the
count of the objects, as well as `$mode.current`.

So if the current mode is `maildir` then:

* `maildir.limit` contains any constraint in-use `all|new|pattern`.
* `maildir.max` contains the (integer) count of messages.
* `maildir.current` contains the index of the currently selected maildir.



Callbacks
---------

The global function `on_idle` is invoked between screen-refreshes.

The global function `on_complete` is called to handle TAB-completion.

Otherwise the most important user-definable functions are:

* Message.to_string()
      * Called to convert a message to something we can display.
* Maildir.to_string()
      * Called to format a Maildir object.
* Message.to_index()
      * Called t format a message for the index-mode.

The latter two functions can return lines with colour settings via:

    $[RED]TEXT TO DISPLAY

Colours include `red`, `blue`, `yellow`, `green`, etc.


Config
------

The Config method is registered automatically and doesn't need to be constructed.

The following methods are available:

* keys()
    * Return all the configuration-keys which have been set.
* get(key)
    * Return the value of a given key.
    * The value might be a string or an array (table of strings with integer indexes).
* set(key,value)
    * Set the value of the given key.
    * The value might be a string or an array (table of strings with integer indexes).

If the function `Config.key_changed` is defined it will be invoked whenever a key has a value updated.

Sample code:

     lumail2 --no-curses --load-file ./config.lua



Files
-----

The following (static) methods exist:

* File:exists(path)
   * Return a boolean based on whether the named file exists.
* File:stat(path)
   * Return a table of information about the named target.
   * Returns `nil` on fialure.


Maildir
-------

Constructor:

    maildir = Maildir.new( "./Maildir" )

The Maildir object has the following methods:

* `path()`
    * Returns the path to the Maildir - what it was constructed with.
* `messages()`
	* Returns an array of Message-objects, one for each message in the maildir.
* `total_messages()`
	* Returns the count of messages in the maildir.
* `unread_messages()`
	* Returns the count of unread/new messages in the maildir.
* `exists`
	* Returns `true` if the Maildir exists.




Message
-------

Constructor:

     message = Message.new( "path/to/message" )

Header access can be achived via `header` to read a single header, or `headers` to return a table of all present headers, and their values.

Flags can be accessed via the `flags()` method, which also allows them to be updated.

The `parts()` method allow a message to be examined, for MIME-parts.

    parts = message:parts()

This returns an array of `MessagePart` objects.  The `MessagePart` object
contains methods:

* type()
    * Returns the content-type of the MIME-part
* content()
	* Returns the content of the part.
* is_attachment()
	* Returns `true` if the part represents an attachment, false otherwise
* filename()
	* Returns the name of the attachment, if `is_attachment` returned true.

See [show_message.lua](show_message.lua) for an example use-case of this method.


Screen
------

The Screen object is registered automatically and doesn't need to be constructed  The following methods are available:

* `clear()`
    * Clear the screan-area.
* `exit()`
    * Exit the main event-loop, and terminate the program.
* `get_line()`
    * Receive a line of input, from the prompt.
* `height()`
    * Return the height of the screen.
* `sleep(N)`
    * Sleep for N-seconds.
* `width()`
    * Return the width of the screen.

Sample code:

     lumail2 --no-curses --load-file ./screen.lua


The screen also has an associated status-panel, hereby referred to as "Panel".  The Panel object has the following methods:

* height()
     * Returns the size of the panel, in lines.
* hide()
     * Hide the panel, if visible.
* show()
     * Show the panel, if hidden.
* text( { "Some text to display", "Goes here" } )
     * Update the text to display.
* text()
     * Retrieve the current text.
* title( "New Title" )
     * Set the title of the panel
* title()
     * Get the title of the panel.
* toggle()
     * Toggle the visibility of the panel.

Sample code:

     lumail2 --load-file ./panel.lua



Utilities
---------

None, at this time.
