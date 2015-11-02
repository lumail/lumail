
lumail2
=======

This repository contains a very primitive "email client".  This is in quotes
because right now the only thing that works is :

* Listing Maildirs.
* Listing Messages within a particular Maildir.
* Viewing a single email-message.
   * Viewing the attachments associated with that message.

There is zero support for:

* Forwarding emails.
* Replying to emails.
* Sending emails.
* Deleting emails.

If you're familiar with the original [lumail project](http://lumail.org/) it should make sense, otherwise it might not.



User-Interface
--------------

The user-interface will be familiar to users of lumail 1.x, the only obvious
changes is the status-panel which can display persistent output, under the control of Lua,
and the addition of new display-modes.

It should be noted that all of the display-modes are created/maintained by Lua code,
which means it is possible to create very flexible and customized output.

Because this is a modal-application you're always in one of a fixed number of modes:


* `maildir`-mode
    * Allows viewing mail folders.
* `index`-mode
    * Allows viewing a list of messages, i.e. the contents of a Maildir.
* `message`-mode
    * Allows you to view a single message.
* `attachment`-mode
    * Which lets you view the attachments associated with a message.
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

For a quick-start you can use the following bindings:

* `TAB` - Toggle the panel
* `M` - Maildir mode
* `I` - Index mode
* `L` - Lua-mode.
* `Q` - Exit


Further Notes
-------------

* [API Documentation](API.md)
   * Documents the Lua classes.
* [Notes on implementation & structure](HACKING.md)
   * See also the [experiments repository](https://github.com/lumail/experiments) where some standalone code has been isolated for testing/learning purposes.
