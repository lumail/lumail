
[![Build Status](https://travis-ci.org/lumail/lumail2.png)](https://travis-ci.org/lumail/lumail2)


lumail2
=======

This repository contains the `lumail2` application, a console-based email
client with fully integrated scripting provided by Lua.

This project is built upon of the previous [Lumail client](https://github.com/lumail/lumail/), and was initiated to improve both the core code and the user-presentation of that code:

* The C++ core is much more consistent.
* The Lua extension support is much more consistent.
* The more things that can be pushed to Lua the better.
    * To allow customization.
    * To allow flexibility.

The current status of the project is that of a work in-progress, but the
core of the project is complete:

* Navigation around the various modes is complete.
* Basic operations may be carried out against email:
     * Viewing Maildir hierarchies.
     * Viewing message-lists.
     * Reading emails.
     * Replying to emails.
     * Forwarding emails.

The missing facilities include:

* Lack of support for GPG/PGP.
* Lack of support for adding attachments to outgoing emails.
* Lack of real documentation.

**NOTE**: Lumail2 may well eat your email, corrupt your email, or
otherwise cause data loss.  If you have no current backups of your
Maildir hierachies you have **NO USE FOR THIS PROJECT**.


User-Interface
--------------

The user-interface will be familiar to users of prior `lumail` release.

The only obvious change, in terms of visual appearance, is the addition of
the status-panel which can display persistent output, under the control of
Lua, and the updated display-modes.

It should be noted that all of the display-modes are created/maintained by
Lua code, which means it is possible to create very flexible and
customized output.

Because this is a modal-application you're always in one of a fixed number
of modes:


* `maildir`-mode
    * Allows viewing lists of Maildir folders.
* `index`-mode
    * Allows viewing a list of messages, i.e. the contents of a Maildir.
* `message`-mode
    * Allows you to view a single message.
    * `attachment`-mode is a submode, and allows you to view the attachments associated with a particular message.
* `lua`-mode.
    * This mode displays output created by Lua.
    * By default it dumps configuration values, & etc.


Building Lumail2
----------------

The code relies upon a small number of libraries:

* lua 5.2.
* libmagic, the file-identification library.
* libgmime-2.6, the MIME-library.
* libncursesw, the console input/graphics library.

Upon a Debian GNU/Linux host, running the Jessie (stable), release the following two commands are sufficient to install the dependencies:

    apt-get install build-essential make pkg-config

    apt-get install liblua5.2-dev libgmime-2.6-dev libncursesw5-dev libpcre3-dev libmagic-dev

With the dependencies installed you should ind the code builds cleanly with:

    make

Running `make install` will install the binary and the [luarocks](https://luarocks.org/) libraries that we bundle.  If you wish to install manually copy the contents of `luarocks.d` to `/etc/lumail2/luarocks.d`.


Using Lumail
------------

When `lumail2` starts it will load each of the following files, in order,
if they exit:

* `/etc/lumail2/lumail2.lua`
* `~/.lumail2/lumail2.lua`

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


Steve
--
