lumail
======

lumail is a modal console-based email client, which has built in support for scripting
via Lua.

The email client is a modal application, which means that you're *always* in one of three
states:

* Interacting with lists of mailboxes.
* Interacting with lists of messages.
* Interacting with a single message.

This suits my email tastes, although I appreciate it perhaps isn't for everybody.


Code
----

The application is developed in C++, and is largely inspired by the proof of concept project
I hacked together in a couple of days:

* cmail
    * https://github.com/skx/cmail/

The dependencies are pretty straightforward:

* lua 5.1 - The scripting language
* ncurses - The console-graphics library.
* mimetic - The MIME-library.

Upon a Debian GNU/Linux system you may install them via:

     # apt-get install libncurses-dev liblua5.1-0-dev lua5.1 libmimetic-dev

Building the code should be as simple as cloning the repository and running "`make`".

From there it may be executed directly, although you might wish to make tweaks to
the supplied configuration file `lumail.lua`.


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

If you examine the supplied [lumail.lua](https://raw.github.com/skx/lumail/master/lumail.lua)
configuration file you'll get a flavour for the configuration.

You will need to point the mail-client at your local Maildir location, from which all
sub-folders will be determined at run-time.

The other configuration largely consists of setting up key-bindings.


Steve
--
