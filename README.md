lumail
======

lumail is a simple console-based email client which is in the process of being
developed.

It is a modal-mailclient, which means that you're always in one of three states:

* Interacting with lists of mailboxes.
* Interacting with lists of messages.
* Interacting with a single message.


Code
----

Code is written in C++, and is largely inspired by the proof of concept project
I hacked together in a couple of days:

* cmail
    * https://github.com/skx/cmail/


Current Status
--------------

Because this mail-client is a modal application the coding has been split into
sections:

* [x] Code the display/manipulation of the Maildir folders.
* [x] Code the display/manipulation of the message-indexes.
* [ ] Code the display/manipulation of a single mail message.

The first two items on this list are complete:

* You can launch the client and navigate/manipulate the Maildir folders.
* Opening a folder, or series of folders, will show you the messages in it.
    * The messages may be scrolled.

The missing step is the ability to read individual messages, reply to messages, etc.


Screenshots
-----------

* [Showing all folders](img/all.png).
* [Showing all folders with new mail](img/new.png).
* [Showing all folders which match a pattern](img/lj.png).


Lua-Primitives
--------------

There are several lua primitives which are useful for interactive use, they include:

* `clear` - Clear the screen.
* `prompt` - Prompt for input, and return it to the caller.
* `msg` - Output a message.
* `quit` - Quit.
* Along with various primitives useful for manipulating the maildir folders, which are documented in `lumail.lua`.


Steve
--
