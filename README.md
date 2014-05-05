[![Build Status](https://travis-ci.org/skx/lumail.png)](https://travis-ci.org/skx/lumail)






Lumail
======

lumail is a modal console-based email client, which features extensive scripting
support, via an embedded Lua intepretter.

You'll find [a quick introduction to using lumail](http://lumail.org/getting-started/) on
the [lumail website](http://lumail.org), along with [screenshots](http://lumail.org/screenshots/).

In brief `lumail` is a modal application, so you're *always* in one of three states:

* Interacting with lists of mailboxes.
* This is the mode you'll start in.
* Default keybindings include:
   * Press `a` to view all mailboxes.
   * Press `n` to view mailboxes containing new mail only.
   * Move around via `j`/`k`/`/`.
   * You may open the single selected folder by pressing `RETURN`.
   * Or you may toggle the selected state by pressing `SPACE` and jumping into index mode with `I` when you've selected all the folders you care about.
* Interacting with lists of messages.  These messages may come from more than one folder.
* Default keybindings include:
   * Press `a` to view all messages.
   * Press `n` to view new messages only.
   * Move around via `j`/`k`/`/`.
* Interacting with a single message.
* Default keybindings include:
   * Move around via `j`/`k`.
   * Delete a message with 'd'.
   * Reply to a message with 'r'.



Building/Installation
---------------------

The application is developed in C++ and has intentionally minimal dependencies:

* lua 5.1 - The scripting language.
    * lua 5.2 is also supported, but is not yet the default.
* libncursesw - The console input/graphics library.
* libgmime-2.6  - The MIME-library.
* libglibmm-2.4 - The library we use for UTF-8 aware strings.
* libpcre3 - The regular-expression library.

Upon a Debian GNU/Linux system you may install all required packages with:

     # apt-get install libncursesw5-dev liblua5.1-0-dev lua5.1 \
         libgmime-2.6-dev libpcre3-dev libglibmm-2.4-dev

> There are [binary packages for Debian GNU/linux](http://packages.steve.org.uk/lumail/), compiled by the author.

Although we might become more complex in the future the code currently builds
via a simple `Makefile`, and running `make` with no arguments should be sufficient.

When you run `make` two versions of the main lumail binary will be built, one with debugging information present, and one without.  Building both at the same time is useful as it allows more diagnostics to be made if/when bugs are found.

There are two choices you can make when building lumail:

* Whether to enable domain-socket support.
     * This allows commands to be sent to a running lumail instance, over a unix domain socket.
     * **NOTE** Even if support is compiled-in there will be no socket by default, instead your configuration file will need to invoke the <a href="http://lumail.org/lua/bind_socket.html">bind_socket()</a> primitive to initiate the listener.
* Which version of Lua to build against (5.1 or 5.2)
     * Run "`make LUA_VERSION=5.1`" or "`make LUA_VERSION=5.2`" to choose explicitly.
     * **NOTE**: For Fedora distributions you will need to run: "`make LUA_VERSION=`".

Once compiled the client may be executed directly, but you will need to supply
a valid (Lua) configuration file:

     $ ./lumail --rcfile ./lumail.lua


Installation should be as simple as copying the supplied configuration file to `/etc/lumail.lua` and copying the binary to `/usr/local/bin`.  If you run `make install` this will be done for you.



Configuration & Lua-Primitives
------------------------------

If you examine the supplied [lumail.lua](https://raw.github.com/skx/lumail/master/lumail.lua) configuration file you'll get a flavour for the configuration.

The main part of the configuration is to point the mail-client at your local
`Maildir` location, from which all sub-folders will be determined at run-time.

At startup the following files are loaded and executed, if they are present:

* `/etc/lumail.lua`
* `/etc/lumail.d/*.lua`
* `~/.lumail/config.lua`

If zero configuration files are loaded then the client will abort with an error
message.  This is to ensure that the keymap(s) are defined, etc.

Once you have configuration file you can use any of the [supplied Lua primitives](http://lumail.org/lua/) to do interesting things.  The [online Lua examples](http://lumail.org/examples/) are a good starting point for reference.


Further Information
-------------------

You may find further information upon the lumail website:

* http://lumail.org/
    * This website is built automatically from the [lumail.org website repository](https://github.com/skx/lumail.org/).


Contributing
------------

I welcome bug reports, and pull-requests.

* There is a [development roadmap](ROADMAP.md) in the repository.
* For coding-style & coding-notes please see [CODING-STYLE](CODING-STYLE).

If you submit two or more functional patches I'm happy to give you direct commit
access to the repository.


Steve
--
