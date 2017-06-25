
[![Build Status](https://travis-ci.org/lumail/lumail.png)](https://travis-ci.org/lumail/lumail)
[![license](https://img.shields.io/github/license/lumail/lumail.svg)]()


lumail
=======

`lumail` is a modern console-based email-client, with fully integrated scripting, implemented in the Lua programming language.

Although primarily developed and tested against GNU/Linux it should run upon Mac OS X, and FreeBSD.  If your system isn't supported, and is Unix-like then this is a bug which should be fixed.

`lumail` is primarily designed to operate against local `Maildir`-hierachies, but IMAP support is available, as well as support for GPG for security.

This `README.md` file contains brief details of the project, with more complete documentation provided on the homepage. The following links on the project website should be a good starting point:

* [Downloading lumail](https://lumail.org/download/)
* [Installing lumail](https://lumail.org/install/)
* [Getting started with lumail](https://lumail.org/getting-started/)
* [The API documentation](https://lumail.org/api/)
   * [Sample Code Snippets](https://lumail.org/examples/).


## Overview

Lumail is a console-based mail client, which is __modal__.  A modal client
means that you're always in one of a small number of states, or modes:

* `maildir`-mode
    * Allows you to see a list of message-folders.
* `index`-mode
    * Allows you to view a list of messages.
       * i.e. The contents of a folder.
* `message`-mode
    * Allows you to view a single message.
       * `attachment`-mode is related, allowing you to view the attachments associated with a particular message.
* `lua`-mode.
    * This mode displays diagnostics and other internal details.
* `keybinding`-mode.
    * Shows you the keybindings which are in-use.
    * Press `H` to enter this mode, and `q` to return from it.


## Compilation & Installation
-----------------------------

Running `make install` will install the binary, the libraries that we bundle, and the perl-utilities which are required for IMAP-operation.

If you wish to install manually then please copy:

* The contents of `lib/` to `/usr/lib/lumail`.
* The contents of `perl.d` to `/usr/share/lumail/`.

You can also see [the notes below](#running-from-git-checkout) about running directly from a `git`-checkout of our repository.  Note that if you wish to [use IMAP](IMAP.md) you'll need to install the extra depedencies for that.


## Configuration

Once installed you'll want to create your own personal configuration file.

To allow smooth upgrades it is __recommended__ you do not edit the global configuration file `/etc/lumail2/lumail2.lua`.  Instead you should copy the sample user-configuration file into place:

      $ mkdir ~/.lumail2/
      $ cp lumail2.user.lua ~/.lumail2/lumail2.lua

If you prefer you can name your configuration file after the hostname of the local system - this is useful if you store your dotfiles under revision control, and share them:

      $ mkdir ~/.lumail2/
      $ cp lumail2.user.lua ~/.lumail2/$(hostname --fqdn).lua

The defaults in [the per-user configuration file](lumail2.user.lua) should be adequately documented, but in-brief you'll want to ensure you set at least the following:

     -- Set the location of your Maildir folders, and your sent-folder
     Config:set( "maildir.prefix", os.getenv( "HOME" ) .. "/Maildir/" );
     Config:set( "global.sent-mail", os.getenv( "HOME" ) .. "/Maildir/sent/" )

     -- Set your outgoing mail-handler, and email-address:
     Config:set( "global.mailer", "/usr/lib/sendmail -t" )
     Config:set( "global.sender", "Some User <steve@example.com>" )

     -- Set your preferred editor
     Config:set( "global.editor", "vim  +/^$ ++1 '+set tw=72'" )



### Running from `git`-checkout

If you wish to run directly from a `git`-checkout you'll need to add some
command-line flags to change the default behaviour:

* Change the location from which Lua libraries are fetched.
* Disable the loading of the global configuration-files.

This can be achieved like so:

     $ ./lumail2 --load-path=$(pwd)/lib/ --no-default --load-file ./lumail2.lua --load-file ./lumail2.user.lua


## Using Lumail

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


## Further Notes

Further documentation can be found upon the [project homepage](https://lumail.org/), but there are also some notes available within this repository:

* [API Documentation](API.md).
   * Documents the Lua classes.
* [Contributor Guide](CONTRIBUTING.md).
* [Notes on IMAP](IMAP.md).
* [Notes on GPG Support](GPG.md).
* [Notes on implementation & structure](HACKING.md).
   * See also the [experiments repository](https://github.com/lumail/experiments) where some standalone code has been isolated for testing/learning purposes.


Steve
--
