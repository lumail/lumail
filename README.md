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

The project is still in the early stages of development, although it is hoped
that it will be fleshed out pretty quickly.

Because this mail-client is a modal application the coding has been split into
sections:

* Code the display/manipulation of the Maildir folders.
* Code the display/manipulation of the message-indexes.
* Code the display/manipulation of a single mail message.

The first of these is complete.  A maildir collection may be searched, browsed,
and toggled.

The display of indexes & messages is still missing.


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
