
lumail2
=======

This is a small repository containing some minimal code for working with Maildir objects, email-messages, and MIME-parts.

If you're familiar with the [lumail](http://lumail.org/) project it should make sense, otherwise it might not.

This is work-in-progress at the moment, significant work needs to be done before this is even remotely useful or interesting to anybody but the author.



User-Interface
--------------

The user-interface I expect to develop will be familiar to users of lumail 1.x,
the only real change is that we now have a status-panel which can display
persistent output, under the control of Lua.


Usage
-----

Lumail is a modal email client, which is expcted to have several major modes:

* A mode for looking at maildir lists.
* A mode for looking at message lists.
* A mode for looking at a single message.
* ..

At the moment there is only a single mode implemented "`demo`" which outputs
`Hello World`.  However this single mode is sufficient to allow us to
prove that things work - specifically that we've wired up keyboard input
correctly, our refresh code works, and that the status-panel works as
expected.

With that in mind you can access the GUI, and execute the client with:

    ./lumail2  --load-file ./lumail2.lua


You can see the [API documentation](API.md) for details of the kind of functions
that you can write, but until we have implemented more of the modes to do
things with useful objects you'll instead need to run the examples like so:

    ./lumail2 --no-curses --load-file ./config.lua
    ./lumail2 --no-curses --load-file ./parts.lua
    ./lumail2 --no-curses --load-file ./show_message.lua
