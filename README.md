
[![Build Status](https://travis-ci.org/lumail/lumail2.png)](https://travis-ci.org/lumail/lumail2)


lumail2
=======

This repository contains the `lumail2` console-based email
client, with fully integrated scripting provided by Lua.

This project is based upon of the previous [Lumail client](https://github.com/lumail/lumail/), and was initiated to improve both the user-interface and the internal implementation:

* The C++ core, and Lua scripting, is much more consistent.
* The more things that can be pushed to Lua the better.
    * To allow customization.
    * To allow flexibility.

The project is perpetually a work in-progress, but the core of the client
is complete and robust:

* All the obvious operations may be carried out:
     * Viewing folder-hierarchies.
     * Viewing the contents of a folder.
     * Reading emails.
     * Replying to emails.
     * Forwarding emails.
     * Composing fresh emails.
     * Deleting emails.

All of these things work against local Maildir-hierarchies __and__ remote
IMAP servers.

**NOTE**: `lumail2` may well eat your email, corrupt your email, or
otherwise cause data loss.  If you have no current backups of your
email you should **NO USE FOR THIS PROJECT**.


User-Interface
--------------

The user-interface will be familiar to users of previous `lumail` project.

The only obvious change, in terms of visual appearance, is the addition of
the status-panel which can display persistent output, under the control of
Lua, and the updated display-modes.

It should be noted that all of the display-modes are created/maintained by
Lua code, which means it is possible to create very flexible and
customized output.

Because this is a modal-application you're always in one of a fixed number
of modes:

* `maildir`-mode
    * Allows viewing lists of mail-folders.
* `index`-mode
    * Allows viewing a list of messages, i.e. the contents of a folder.
* `message`-mode
    * Allows you to view a single message.
    * `attachment`-mode is a submode, and allows you to view the attachments associated with a particular message.
* `lua`-mode.
    * This mode displays output created by Lua.
    * By default it dumps configuration values, & etc.


Building Lumail2
----------------

The core of the project relies upon a small number of libraries:

* lua 5.2.
* libmagic, the file-identification library.
* libgmime-2.6, the MIME-library.
* libncursesw, the console input/graphics library.

Upon a Debian GNU/Linux host, running the Jessie (stable) release, the following two commands are sufficient to install the dependencies:

    apt-get install build-essential make pkg-config

    apt-get install liblua5.2-dev libgmime-2.6-dev  \
       libncursesw5-dev libpcre3-dev libmagic-dev


With the dependencies installed you should find the code builds cleanly with:

    $ make

The integrated test-suite can be executed by running:

    $ make test



Installing lumail2
------------------

Running `make install` will install the binary and the [luarocks](https://luarocks.org/) libraries that we bundle, along with the perl-utiities which are required for IMAP-operation.

If you wish to install manually copy:

* The contents of `luarocks.d` to `/etc/lumail2/luarocks.d`.
* The contents of `perl.d` to `/etc/lumail2/perl.d`.

NOTE: If you wish to use IMAP you'll need to install the two perl modules `JSON` and `Net::IMAP::Client`.  Upon a Debian GNU/Linux system this can be done
via:

     apt-get install libnet-imap-client-perl libjson-perl




Configuring Lumail
-------------------

When `lumail2` starts it will load each of the following files:

* `/etc/lumail2/lumail2.lua`
    * This will then load `~/.lumail2/$HOSTNAME.lua` if present.
* `~/.lumail2/lumail2.lua`

The intention is that you will always run `make install` to ensure
that the global file is present, and you will then place your own
configuration in the file `~/.lumail2/lumail2.lua`.

If you keep your personal configuration settings beneath `~/.lumail2/`
then they will remain effective if/when you ever upgrade lumail.

The following settings are probably the minimum you'll require,
given the sensible defaults in the global configuration file:


     -- Set the location of your Maildir folders, and your sent-folder
     Config:set( "maildir.prefix", os.getenv( "HOME" ) .. "/Maildir/" );
     Config:set( "global.sent-mail", os.getenv( "HOME" ) .. "/Maildir/sent/" )

     -- Set your outgoing mail-handler, and email-address:
     Config:set( "global.mailer", "/usr/lib/sendmail -t" )
     Config:set( "global.sender", "Some User <steve@example.com>" )

     -- Set your editor
     Config:set( "global.editor", "vim  +/^$ ++1 '+set tw=72'" )

Other options are possible, and you'll find if you wish to
[use IMAP](IMAP.md) you need some more options.  For more details
please do read the [sample lumail2.lua file](lumail2.lua).



Using Lumail2
-------------

By default you'll be in the `maildir`-mode, and you can navigate with `j`/`k`, and select items with `ENTER`.

For a quick-start you can use the following bindings:

* `TAB` - Toggle the display of the status-panel.
* `P` - Toggle the size of the panel.
* `M` - Maildir mode.
* `I` - Index mode.
* `L` - Lua-mode.
* `Q` - Exit.


Further Notes
-------------

* [API Documentation](API.md)
   * Documents the Lua classes.
* [Contributor Guide](CONTRIBUTING.md)
* [Notes on IMAP](IMAP.md)
* [Notes on implementation & structure](HACKING.md)
   * See also the [experiments repository](https://github.com/lumail/experiments) where some standalone code has been isolated for testing/learning purposes.


Steve
--
