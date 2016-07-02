Lumail GPG Support
==================

There is (experimental) support for handling GPG with
Lumail.

GPG support is implemented via executing the `mimegpg`
binary, so you'll need that installed.

With the right binary installed there is no further
configuration required for lumail - though you will
need to ensure you've got a GPG-agent running, and
your key is unlocked before you start.


Incoming Email
--------------

GPG is supported for incoming messages at the moment:

* Working
    * You can validate signed messages.
    * You can decrypt outgoing messages.


Outgoing Email
--------------

You can choose to sign your outgoing messages,
encrypt your outgoing messages, or both.

In the case of encryption the recipient's email
will be used to determine the key to use.


