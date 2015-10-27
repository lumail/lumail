
lumail2
=======

This repository contains a very primitive "email client".  This is in quotes because right now the only thing that works is :

* Listing Maildirs.
* Listing Messages within a particular Maildir.
* Viewing a single email-message.

There is zero support for:

* Forwarding emails.
* Replying to emails.
* Sending emails.
* Deleting emails.

If you're familiar with the original [lumail project](http://lumail.org/) it should make sense, otherwise it might not.



User-Interface
--------------

The user-interface I expect to develop will be familiar to users of lumail 1.x,
the only real change is that we now have a status-panel which can display
persistent output, under the control of Lua.

Lumail is a modal client, which means you're always in one of a fixed number of modes:

* `maildir`-mode
    * Allows viewing mail folders.
* `index`-mode
    * Allows viewing a list of messages, i.e. the contents of a Maildir.
* `demo`-mode.
    * Shows some simple animation.
* `lua`-mode.
    * Is a mode that displays output created by Lua.
    * This is the way that we allow you to write custom output.



Using Lumail
------------

The first step is obviously to compile the code, and then execute the
client itself:

    make
    ./lumail2

When the program starts it will load each of the following files, in order,
if they exit:

* `/etc/lumail2/lumail2.lua`
* `~/.lumail2/lumail2.lua`
* `./lumail2.lua`
     * This will be removed before the first release - it is a security-hole.

Once you've done that you'll be in the `maildir`-mode, and you can
navigate with `j`/`k`, and view the contents of a maildir via `return`.

For quick use you can use:

* `TAB` - Toggle the panel
* `M` - Maildir mode
* `I` - Index mode
* `L` - Lua-mode.
* `D` - Demo-mode.
* `Q` - Exit


Finally you may see the [API documentation](API.md) for details of the kind
of functions that you can write, but until we have implemented more of the
modes to do things with useful objects you'll instead need to run the
examples like so:

    ./lumail2 --no-curses --load-file ./config.lua
    ./lumail2 --no-curses --load-file ./parts.lua
    ./lumail2 --no-curses --load-file ./show_message.lua
    ./lumail2 --no-curses --load-file ./dump_maildir.lua
