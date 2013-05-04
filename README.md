lumail
======

lumail is a simple console-based email client which is in the process of being
developed.

It is a modal-mailclient, which means that you're always in one of three states:

* Interacting with lists of mailboxes.
* Interacting with lists of messages.
* Interacting with a single message.


Code
----

Code is written in C++, and is largely inspired by the proof of concept project
I hacked together in a couple of days:

* cmail
    * https://github.com/skx/cmail/


Current Status
--------------

The project is still in the early stages of development, although it is hoped
that it will be fleshed out pretty quickly.

The user-interface is currently a blank-screen, although we have an extensible
mechanism for evaluating Lua-code, and responding to keypresses.

There are several lua primitives which are useful for interactive use, they include:

* `clear` - Clear the screen.
* `prompt` - Prompt for input, and return it to the caller.
* `msg` - Output a message.
* `quit` - Quit.

This section of the project notes will be updated once directories may be browsed
and similar.  At the moment I'm laying the foundation with the primitives for
the low-level and the lua-bindings being written.

Once the primitives are in place then the actual application should be quickly
created.


Steve
--
