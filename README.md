lumail
======

lumail is a modal console-based email client, which has built in support for scripting
via Lua.

The email client is a modal application, which means that you're *always* in one of three
states:

* Interacting with lists of mailboxes.
* Interacting with lists of messages.
* Interacting with a single message.

This suits my email tastes, although perhaps isn't for everybody.

Once launched you'll start in the "maildir" mode.  This consists of a scrollable list
of folder-names.  You can page up/down with `j/k`, or search with `/`.

Each maildir-name has a small `[ ]` next to it.  This indicates whether the folder is
selected or not.  Unlike other mail-clients where you open a mailbox/folder with lumail
you can open __multiple__ folders at the same time.

To open the single folder which is highlighted, press RETURN.  Otherwise you may toggle
the folders selected state with "SPACE".  Once you've selected as many folders as you
wish you can press "I" to view the index-mode.

Index-mode is navigated the same way as maildir-mode; use `j + k + /` to move around,
then press "SPACE"/"RETURN" to view an individual message.


Code
----

The application is developed in C++, and is largely inspired by [the proof of concept code](https://github.com/skx/cmail/) I hacked together in a couple of days.

The dependencies are pretty straightforward:

* lua 5.1 - The scripting language
* ncurses - The console-graphics library.
* mimetic - The MIME-library.

Upon a Debian GNU/Linux system you may install them all via:

     # apt-get install libncurses-dev liblua5.1-0-dev lua5.1 libmimetic-dev

Building the code should be as simple as cloning this repository and running "`make`".

From there it may be executed directly, although you might wish to make tweaks to
the supplied configuration file `lumail.lua`.


Current Status
--------------

Because lumail is modal application the coding has been split into sections:

* Code the display/manipulation of the Maildir folders.
    * This is complete.
    * You may scroll/search/limit the display of folders.
* Code the display/manipulation of the message-indexes.
    * This is functional.
    * You may scroll/search/limit the display of folders.  But such searches are slow.
* Code the display/manipulation of a single mail message.
    * This is nearly-functional.
    * You may view the first screen-ful of a message.  If it is text/plain.

Missing functionality largely relates to using this client for real.

The following features are missing:

* The ability to compose a new message.
* The ability to reply to a message.
* The ability to delete a message.
* The ability to mark a new message as read.


Screenshots
-----------

* [Showing all folders](img/all.png).
* [Showing all folders with new mail](img/new.png).
* [Showing all folders which match a pattern](img/lj.png).


Configuration & Lua-Primitives
------------------------------

If you examine the supplied [lumail.lua](https://raw.github.com/skx/lumail/master/lumail.lua)
configuration file you'll get a flavour for the configuration.

The main part of the configuration is to  point the mail-client at your local Maildir
location, from which all sub-folders will be determined at run-time.

At startup the following three lua files are evaluated, if present:

* `/etc/lumail.lua`
* `~/.lumail/config.lua`
* `./lumail.lua`
   * This is depreciated and will be removed in the future.


Further Information
-------------------

You may find further information upon the lumail website:

* http://lumail.org/
    * This website is built automatically from the [lumail.org repository](https://github.com/skx/lumail.org/).


Steve
--
