
[![Build Status](https://travis-ci.org/lumail/lumail2.png)](https://travis-ci.org/lumail/lumail2)
[![license](https://img.shields.io/github/license/lumail/lumail2.svg)]()


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

Each of the operations works against both local-maildir hierarchies,
and [remote IMAP servers](IMAP.md).


**NOTE**: `lumail2` may well eat your email, corrupt your email, or
otherwise cause data loss.  If you have no current backups of your
email you should **NOT USE THIS PROJECT**.


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
    * Allows you to see a list of message-folders.
* `index`-mode
    * Allows you to view a list of messages.
    * i.e. The contents of a folder.
* `message`-mode
    * Allows you to view a single message.
    * `attachment`-mode is a submode, allowing you to view the attachments associated with a particular message.
* `lua`-mode.
    * This mode displays output created by Lua.
    * By default it dumps configuration values, & etc.
* `keybinding`-mode.
    * Shows you the keybindings which are in-use.
    * Press `H` to enter this mode, and `q` to return from it.


Building Lumail2
----------------

The core of the project relies upon a small number of libraries:

* lua 5.2.
* libmagic, the file-identification library.
* libgmime-2.6, the MIME-library.
* libncursesw, the console input/graphics library.

### Linux
Upon a Debian GNU/Linux host, running the Jessie (stable) release, the following command is sufficient to install the required dependencies:

     apt-get install build-essential libgmime-2.6-dev liblua5.2-dev libmagic-dev libncursesw5-dev libpcre3-dev make pkg-config


With the dependencies installed you should find the code builds cleanly with:

    $ make

The integrated test-suite can be executed by running:

    $ make test


### OS X

Make sure Xcode is installed and you probably want a package manager like [brew](http://brew.sh/) to install the required dependencies:

     brew install lua gmime libmagic ncurses pkg-config

The code can then be built as follows:

    $ PKG_CONFIG_PATH=/usr/local/Cellar/ncurses/6.0_1/lib/pkgconfig make


Installation and Configuration
------------------------------

Running `make install` will install the binary, the libraries that we bundle, and the perl-utilities which are required for IMAP-operation.

If you wish to install manually copy:

* The contents of `lib/` to `/etc/lumail2/lib`.
* The contents of `perl.d` to `/etc/lumail2/perl.d`.

**NOTE**: If you wish to use IMAP you'll need to install the two perl modules `JSON` and `Net::IMAP::Client`.  Upon a Debian GNU/Linux system this can be done
via:

     apt-get install libnet-imap-client-perl libjson-perl

Once installed you'll want to create your own personal configuration file.

To allow smooth upgrades it is __recommended__ you do not edit the global configuration file `/etc/lumail2/lumail2.lua`.  Instead you should copy the sample user-configuration file into place:

      $ mkdir ~/.lumail2/
      $ cp lumail2.user.lua ~/.lumail2/lumail2.lua

If you prefer you can name your configuration file after the hostname of the local system - this is useful if you store your dotfiles under revision control, and share them:

      $ mkdir ~/.lumail2/
      $ cp lumail2.user.lua ~/.lumail2/$(hostname --fqdn).lua

The defaults in [the per-user configuration file](lumail2.user.lua) should be adequately
documented, but in-brief you'll want to ensure you set at least the following:

     -- Set the location of your Maildir folders, and your sent-folder
     Config:set( "maildir.prefix", os.getenv( "HOME" ) .. "/Maildir/" );
     Config:set( "global.sent-mail", os.getenv( "HOME" ) .. "/Maildir/sent/" )

     -- Set your outgoing mail-handler, and email-address:
     Config:set( "global.mailer", "/usr/lib/sendmail -t" )
     Config:set( "global.sender", "Some User <steve@example.com>" )

     -- Set your preferred editor
     Config:set( "global.editor", "vim  +/^$ ++1 '+set tw=72'" )

Other options are possible, and you'll find if you wish to [use IMAP](IMAP.md) you need some more options.  If you wish to use encryption you should also read the [GPG notes](GPG.md).


Using Lumail2
-------------

By default you'll be in the `maildir`-mode, and you can navigate with `j`/`k`, and select items with `ENTER`.

For a quick-start you can use the following bindings:

* `TAB` - Toggle the display of the status-panel.
   * The panel displays brief messages when "things" happen.
   * `P` - Toggle the size of the panel.
   * `ctrl-p` enters you into a mode were you can view/scroll through past messages.
* `H` - Shows the keybindings which are configured.
* `M` - See your list of folders.
* `q` - Always takes you out of the current mode and into the previous one.
   * Stopping at the folder-list (`maildir`-mode).
* `Q` - Exit.


Further Notes
-------------

* [API Documentation](API.md).
   * Documents the Lua classes.
* [Contributor Guide](CONTRIBUTING.md).
* [Notes on IMAP](IMAP.md).
* [Notes on GPG Support](GPG.md).
* [Notes on implementation & structure](HACKING.md).
   * See also the [experiments repository](https://github.com/lumail/experiments) where some standalone code has been isolated for testing/learning purposes.


Steve
--
