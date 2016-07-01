Lumail GPG Support
==================

GPG is supported for the verification of messages, but nothing else at
the moment:

* You can validate signed messages.

* You cannot sign outgoing messages.

* You cannot encrypt outgoing messages.

* You cannot decrypt outgoing messages.


Configuration
-------------

There is no configuration required, provided you have the `mimegpg`
binary installed upon your system.

If `mimegpg` is detected then incoming messages which are signed
will be verified automatically.
