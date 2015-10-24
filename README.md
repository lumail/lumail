
lumail2
=======

This is a small repository containing some minimal code for working with Maildir objects, email-messages, and MIME-parts.

If you're familiar with the [lumail](http://lumail.org/) project it should make sense, otherwise it might not.


Overview
--------

The code is basic at the moment with none of the niceties that we'd actually implement if we were going further.

That said we do have a consistent and reliable way of handling objects, and a decent pattern for adding future objects.



Usage
-----

Compile with `make`, then run a lua-script by running:

    ./lumail2 --load-file ./path/to/script.lua

Examples are found beneath `./test/` for example:

    shelob ~/lumail2 $ make test
    for i in test/*.lua; do ./lumail $i ; done
    OK. We decoded the From-header.
    OK 1 - Read 'From' header
    OK 2 - Read 'From' header
    OK 3 - Read 'From' header
    OK 4 - Read 'From' header
    OK 5 - Headers all have identical contents.
    OK 1 - Read flags from message path.
    OK 2 - Read new-flag from message path.
    OK 3 - Read new-flags from message path.
