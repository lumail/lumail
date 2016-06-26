# Lua API


The core idea behind lumail2, and the reason for writing it rather than
continuing to extend the legacy-project, is that all the items that are
involved are Objects.

When you're writing configuration functions you should only be dealing
with simple objects, such as:

* Configuration values.
* Directories.
* Files.
* Maildirs.
* Messages.
    * Message Parts - i.e. attachments, or inline MIME-parts.
* Networking.
* Regular Expressions.
* The screen.
    * The status-panel, which is optionally displayed upon the screen.

By making all the basic things objects, and simplifying our API we've
got a mail-client which is simple to script, and has a unified feel.
There is no need for a large number of global-functions, just a few
types of objects and ways to create/access them.


### Callbacks

The mail-client is written in C++ and generally defers to the Lua
code to perform actions.

There are some cases though where Lua is specifically invoked from the C++
core in the style of a traditionally callback:

* `on_error(msg)`
     * This function is called when an error is encountered.
* `on_complete(token)`
     * This is called when the user invokes TAB-completion upon a token.
     * It should return a table of matches, which are presented to the user.
* `on_idle()`
     * This function is called regularly from the main loop.
     * You can perform background action here.
* The various `_view()` functions.
     * There is a Lua function for each of our modes, for example `attachment_view()`, `index_view()`, etc.

Previously we had many more callbacks, for example a function that could be
invoked when you opened a message, or changed the folder-selection. These
days a lot of that code is already implemented in Lua, so if you wish to
add code to handle such changes you can add it directly.



### Config

The `Config` object allows you to get, set, and iterate over configuration values.

A configuration variable will have both a name and a value, and we support the use of several different types for the value:

* an integer
* a string
* a table of strings.

The following (static) methods are available:

* `Config:keys()`
    * Return all the configuration-keys which have been set.
* `Config:get(key)`
    * Return the value of a given key.
    * The value might be an integer, a string or an array (table of strings with integer indexes).
* `Config:set(key,value)`
    * Set the value of the given key.
    * The value might be an integer, a string or an array (table of strings with integer indexes).

If the function `Config:key_changed` is defined it will be invoked whenever
the value of a key is changed.  The single argument being the name of the key
which has been updated.

**NOTE**: We've defined a helper method in `lumail2.lua` which allows you to retrieve the value of a configuration-key and return a default value if the key is not set.

      function Config.get_with_default(key,default)
          ..
      end

Sample code is available in `sample.lua/config.lua`.



### Configuration Variables

We have a number of variables which are special, the most important ones are:

* `colour.unread`
    * The colour to use when drawing unread-messages.
    * The colour to use when drawing maildirs containing unread-messages.
* `maildir.prefix`
    * This holds the prefix to the maildir hierarchy.
    * Maildirs are (recursively) found from here.
* `index.fast`
    * If this is set to 1 we'll only format messages which are _visible_ when opening folders.
    * This is a speed optimization for large Maildirs, or when using IMAP.
* `index.sort`
    * The method to sort messages by: `date`, `file`, `from`, `none`, or `subject` at this time.
    * Sorting is documented below.
* `global.editor`
    * The user's editor.
* `global.from`
    * The email address to send messages from.
* `global.timeout`
    * The timeout period (milliseconds) in our event-loop.
* `global.tmpdir`
    * The directory to use for temporary files - defaults to "/tmp".
* `global.history`
    * The name of the file to write input-history to.
* `global.horizontal`
    * The horizontal-offset used to implement left/right scrolling.
* `global.mode`
    * This holds the name of the currently active display-mode
* `global.sent-mail`
    * This is the Maildir to which outgoing messages are saved.
* `global.over-draw`
    * If this is set to `1` then an extra line will be drawn at the bottom of the screen to mask any misdraw-artifacts.

For each mode that has a display there will be a `$mode.max` to store the
count of the objects, as well as `$mode.current`.

So if the current mode is `maildir` then:

* `maildir.limit` contains any constraint in-use `all|new|pattern`.
* `maildir.max` contains the (integer) count of maildirs.
* `maildir.current` contains the index of the currently selected maildir.

The global `ARGS` table contains all arguments passed to the command-line,
and can be used for your own purposes.

For day to day usage some variables change the way that the client
displays things, these are:

* `message.all_parts`
    * Alternate between showing some/all text-parts in message-mode.
* `message.headers`
    * Alternate between showing some/all headers in message-mode.
* `message.prepend`
    * Specifies whether additional MIME-parts should be appended/prepended to the display.
* `maildir.truncate`
    * Alternate between showing the full/truncated Maildir path in maildir-mode.


### Sorting Messages

The sorting of messages is implemented in C++, but uses the Lua
functionality to ensure the user can influence the behaviour.

The sorting method is stored in the variable `index.sort`, which
will select the appropriate Lua callback function to perform the sorting.

For example:

* If `index.sort` is set to `date`.
    * Sorting calls `compare_by_date`.
* If `index.sort` is set to `from`.
    * Sorting calls `compare_by_from`.

(i.e. "compare_by_XXX" is invoked when `index.sort` is `XXX`.)

To define your local sorting solution you should:

* Set `index.sort` to `local`.
* Implement `function compare_by_local()`.





### Directories

The following (static) methods exist:

* `Directory:entries(path)`
   * Return a table of file-entries, present beneath the named directory.
* `Directory:exists(path)`
   * Return `true` if the given directory exists.
   * Return `false` otherwise.
* `Directory:mkdir(path)`
   * Create the given directory, including all parent-parts.
* `Directory:is_maildir(path)`
   * Return `true` if the given directory is a Maildir.
   * Return `false` otherwise.

Sample code is available in `sample.lua/file.lua`.



### Files

The following (static) methods exist:

* `File:basename(path)`
   * Return the basename of the given path.
* `File:copy(src,dest)`
   * Copy the given source file to the specified destination.
* `File:exists(path)`
   * Return a boolean based on whether the named file exists.
* `File:stat(path)`
   * Return a table of information about the named target.
   * Returns `nil` on failure.

Sample code is available in `sample.lua/file.lua`.



### Global State

There are some things which are global, and these largely revolve around
available maildirs, and messages.

There is the notion that a message might be selected, and similarly a
maildir.

The following API methods are available to help you with this:

* `Global:maildirs()`
     * Retrieve the list of available maildirs.
* `Global:current_maildir()`
     * Retrieve the currently-selected maildir.
* `Global:select_maildir(mdir)`
     * Set the specified Maildir as current.
* `Global:current_message()`
     * Retrieve the currently-selected message.
* `Global:current_messages()`
     * Retrieve the currently-available messages.
     * This pays attention to the `index.limit` variable.
* `Global:select_message(msg)`
     * Set the specified Message as current.
* `Global:sort_messages(tbl)
     * Return the given table of message, sorted according to `index.sort`.


### Logfile Usage

There is a simple primitive for writing messages to a logfile, which
can be useful for debugging.

By default log-entries are not written, to enable them you must set
a filename like so:

     Config:set( "global.logfile" , "/path/to/log" )

Once a filename has been set logs may be added via:

     Log:append( os.date() .. " Hello!" )
     Log:append( os.date() .. " Goodbye!" )


### Maildir

You can gain access to Maildir objects in several ways:

* Constructing it manually: `m = Maildir.new( "./Maildir" )`.
* Calling `Global:maildirs()` to get a list of all available Maildirs.
* Calling `Global:current_maildir()` to return the currently selected maildir.
    * This returns `nil` if no maildir is currently selected.

The Maildir object has the following methods:

* `is_imap()`
    * Returns true if this maildir represents a __remote__ IMAP folder.
* `is_maildir()`
    * Returns true if this maildir represents a __local__ Maildir folder.
* `path()`
    * Returns the path to the Maildir - what it was constructed with.
* `messages()`
	* Returns an array of Message-objects, one for each message in the maildir.
* `mtime()`
    * Return the modified time of the given maildir, as seconds past the epoch.
* `save_message(msg)`
    * Save the specified message to this maildir.
* `total_messages()`
	* Returns the count of messages in the maildir.
* `unread_messages()`
	* Returns the count of unread/new messages in the maildir.
* `exists`
	* Returns `true` if the Maildir exists.



### Message

The Message object represents a single message, contained within a maildir.

You can get access to message objects in several ways:

* Construct one manually via `Message.new( path/to/message )`.
* Call the `messages()` method on the currently-selected Maildir.
    * This will return all messages in the given maildir, whether new, old or something else.
* Call the `Global:current_message()` method.
    * This returns the single currently-selected message, if any.
    * If no message is selected it will return `nil`.
* Call the `Global:current_messages()` method.
    * This returns the currently available messages.
    * This pays attention to the `index.limit` variable.

Message methods:

* `add_attachments(table)`
   * This method allows attachments to be added to a _vanilla_ email.
   * **NOTE**: Adding attachments to a message already containing attachment-parts will result in corruption.  This is designed solely for use when composing outgoing messages.
   * Sample code is available in `sample.lua/add_attachment.lua`.
* `flags()`
   * Get the flags for the message.
* `flags(new_flags)`
   * Update the flags for the message.
* `generate_message_id()`
   * Generate a random message-ID suitable for use in an email.
* `header(name)`
   * Return the content of the named header, e.g. "Subject".
* `headers()`
   * Return the names and values of every known-header, as a table.
   * **NOTE**: All header-names are lower-cased.
* `mark_read()`
   * Mark the message as having been read.
* `mark_unread()`
   * Mark the message as not having been read.
* `mtime()`
   * Return the modified time of the message, as seconds past the epoch.
* `parts()`
   * Get the MIME-parts of the message, as a table.
* `path()`
   * Return the path to the message, on-disk.


#### Message-Parts

MessagePart objects are returned from the `parts()` method.  The `MessagePart`
object contains the following methods:

* `children()`
    * Returns any MessagePart children this part might have.
* `content()`
	* Returns the content of the part.
* `is_attachment()`
	* Returns `true` if the part represents an attachment, false otherwise.
* `filename()`
	* Returns the name of the attachment, if `is_attachment` returned true.
* `parent()`
    * Returns the parent of the specified message-part, if any.
    * This returns `nil` if the part is not a child.
* `size()`
    * Return the size of the content.
* `type()`
    * Returns the content-type of the MIME-part.

Sample code is available in `sample.lua/show_message.lua`.



### MIME-Type

There is a simple helper-object allowing you to retrieve the MIME-type
of files, based upon their contents.   This lookup is achieved via
`libmagic`.

* `MIME:type(file)`
     * Return the MIME-type of the specified file.

Sample code is available in `sample.lua/mime_type.lua`.



### Networking

There is only a single networking method:

* `Net:hostname()`
     * Return the FQDN of the current system.

Sample code is available in `sample.lua/net.lua`.



### Regular Expressions

There is a thin wrapper around PCRE for those who prefer this family of
regular expressions.  The single method is:

* `Regexp:match(pattern, string)`.

The return value will vary depending on the regexp:

* If the pattern contains no capture-groups then it will return `true`, or `false`.
* If the pattern contains capture groups then it will return a table containing any matches.

Sample code is available under `sample.code/regexp.lua`.



### Screen

The Screen object is registered automatically and doesn't need to be constructed  The following (static) methods are available:

* `Screen:clear()`
    * Clear the screan-area.
* `Screen:execute(cmd)`
    * Execute the given command, resetting the screen first.
* `Screen:exit()`
    * Exit the main event-loop, and terminate the program.
* `Screen:get_line( prompt-string, default-input )`
    * Receive a line of input, from the prompt.
* `Screen:height()`
    * Return the height of the screen.
* `Screen:prompt("Text", "chars" )`
    * Accept input from a small list of characters, used for showing menus, etc.
* `Screen:sleep(N)`
    * Sleep for N-seconds.
* `Screen:stuff(txt)`
    * Add text to the pending input-queue, to fake user-input.
    * For example: `Screen:stuff( ":Panel:append('ok');\n" )`
* `Screen:width()`
    * Return the width of the screen.

Sample code is available in `sample.lua/screen.lua`.

#### The Panel

The screen has an associated status-panel, hereby referred to as the panel.

The panel is designed to collect & contain output messages for the user.
New entries may be appended to it at any time, and the most recent N entries
are displayed - the number being dependent upon the height of the panel.

The Panel object has the following (static) methods:

* `Panel:append(str)`
     * Append the specified string to the panel area.
* `Panel:clear()`
     * Remove all prior output.
* `Panel:height()`
     * Get or set the size of the panel, in lines.
     * The panel will always be at least six lines tall.
* `Panel:hide()`
     * Hide the panel, if visible.
* `Panel:show()`
     * Show the panel, if hidden.
* `Panel:text()`
     * Retrieve the current text in the panel, as a table.
* `Panel:title( "New Title" )`
     * Set the title of the panel
* `Panel:title()`
     * Get the title of the panel.
* `Panel:toggle()`
     * Toggle the visibility of the panel.

Sample code is available in `sample.lua/panel.lua`.



### Views

Each of the major modes is implemented in a combination of Lua and C++.

On the C++ side there is a virtual class instantiated which has the following
two methods:

* `draw()`
   * Draw the text.
* `on_idle()`
   * Called to make updates, if required.

On the Lua side each mode will call a method like:

* `index_view()`
    * Get the text to display in index-mode
* `lua_view()`
    * Get the text to display in lua-mode
* `message_view()`
    * Get the text to display in message-mode
* `maildir_view()`
    * Get the text to display in maildir-mode


These methods must return a table of lines, which will then be displayed.
The lines may contain a prefix containing colour information.  For example:

    function lua_view()
      local r = { "$[RED]This is red",
                  "$[YELLOW]This is yellow!" }
      return r
    end

The function `lua_view` generates the output used in Lua-mode, one of
the many available modal view-modes.
