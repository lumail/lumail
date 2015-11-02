API
===

The core idea of lumail2, and the reason for writing it rather than continuing to extend
the version one project, is that everything is an object.

At the moment we have objects for working with:

* Configuration variables.
* Directories
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



Directories
-----------

The following (static) methods exist:

* Directory:entries(path)
   * Return a table of file-entries, present beneath the named directory.
* Directory:exists(path)
   * Return `true` if the given directory exists.

Sample code:

     lumail2 --no-curses --load-file ./file.lua


Files
-----

The following (static) methods exist:

* File:exists(path)
   * Return a boolean based on whether the named file exists.
* File:stat(path)
   * Return a table of information about the named target.
   * Returns `nil` on failure.


Sample code:

     lumail2 --no-curses --load-file ./file.lua


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

The currently visible maildirs can be retrieved via `current_maildirs()`.



Message
-------

Teh Message object represents a single message, contained within a maildir.

You can get access to message objects in three ways:

* Construct it manually via `Message.new( path/to/message )`.
* If you're in `message`-mode the current message can be found via `current_message()`.
* If you're in `maildir`-mode the list of available messages may be retrieved via `messages()`.


Message methods:

* `flags`
   * Get/Set the flags for the message.
* `header`
   * Return the content of the named header, e.g. "Subject".
* `headers()`
   * Return the names and values of every known-header, as a table.
* `mark_read()`
   * Mark the message as having been read.
* `mark_unread()`
   * Mark the message as not having been read.
* `parts()`
   * Get the MIME-parts of the message, as a table.
* `path()`
   * Return the path to the message, on-disk.

MessagePart objects are returned from the `parts()` method.  The `MessagePart`
object contains the following methods:

* type()
    * Returns the content-type of the MIME-part.
* content()
	* Returns the content of the part.
* is_attachment()
	* Returns `true` if the part represents an attachment, false otherwise.
* filename()
	* Returns the name of the attachment, if `is_attachment` returned true.

See [show_message.lua](show_message.lua) for an example use-case of this method.


Screen
------

The Screen object is registered automatically and doesn't need to be constructed  The following methods are available:

* `clear()`
    * Clear the screan-area.
* `execute(cmd)`
    * Execute the given command, resetting the screen first.
* `exit()`
    * Exit the main event-loop, and terminate the program.
* `get_line( prompt-string )`
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



Views
-----

Each of the major modes is implemented in a combination of Lua and C++.

On the C++ side there is a virtual class instantiated which has the following
two methods:

* draw()
   * Draw the text.
* on_idle()
   * Called to make updates, if required.

On the Lua side each mode will call a method like:

* index_view()
    * Get the text to display in index-mode
* lua_view()
    * Get the text to display in lua-mode
* message_view()
    * Get the text to display in message-mode
* maildir_view()
    * Get the text to display in maildir-mode


These methods must return a table of lines, which will then be displayed.
The lines may contain a prefix containing colour information.  For example:

    function lua_view()
      local r = { "$[RED]This is red",
                  "$[YELLOW]This is yelloW!" }
      return r
    end



Utilities
---------

None, at this time.
