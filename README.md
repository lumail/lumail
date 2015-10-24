
lumail2
=======

This is a small repository containing some minimal code for working with Maildir objects, email-messages, and MIME-parts.

If you're familiar with the [lumail](http://lumail.org/) project it should make sense, otherwise it might not.

This is work-in-progress at the moment, significant work needs to be done before this is even remotely useful or interesting to anybody but the author.



User-Interface
--------------

The user-interface I expect to develop will be familiar to users of lumail 1.x,
but we do expect to see a toggleable status-panel.

This is demonstrated in the example:

        cd misc/
        make
        ./ui-demo


Usage
-----

At the moment there is no useable user-interface, just a driver script
which loads a bunch of objects and API-methods from the embedded code.

You can see the [API documentation](API.md) for details, but for the moment
the only useful thing you can do is execute simple scripts.  For example:

    ./lumail2 --no-curses --load-file ./config.lua
    ./lumail2 --no-curses --load-file ./parts.lua
    ./lumail2 --no-curses --load-file ./show_message.lua

You'll need to read the [API documentation](API.md) to see what you can do,
until we have a workable user-interface - that is Steve's current priority.

