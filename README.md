
lumail2
=======

This repository contains a work-in-progress email client, designed for console use,
with fully integrated Lua scripting support.

This project is a reimaginging of the previous [Lumail client](https://github.com/lumail/lumail/),
which was initiated to improve both the core code and the user-presentation of that code:

* The C++ core is much more consistent.
* The Lua extension support is much more consistent.
* The more things that can be pusehd to Lua the better. 
    * To allow customization.
    * To allow flexibility.

The current status of the project is that navigation works across various modes (as `lumail` is
a modal client), and there are primitives for composing mail, replying to mail, and forwarding
mail.

Missing support at the moment is for things like:

* Deleting emails.
* Sorting emails.


User-Interface
--------------

The user-interface will be familiar to users of lumail 1.x, the only obvious
change is the addition of the status-panel which can display persistent output,
under the control of Lua, and the addition of new display-modes.

It should be noted that all of the display-modes are created/maintained by Lua code,
which means it is possible to create very flexible and customized output.

Because this is a modal-application you're always in one of a fixed number of modes:


* `maildir`-mode
    * Allows viewing mail folders.
* `index`-mode
    * Allows viewing a list of messages, i.e. the contents of a Maildir.
* `message`-mode
    * Allows you to view a single message.
    * `attachment`-mode is a submode, and allows you to view attachments associated with a message.
* `lua`-mode.
    * This mode displays output created by Lua.
    * By default it dumps configuration values, & etc.



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
navigate with `j`/`k`, and select items with `enter`.

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
