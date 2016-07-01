Lumail GPG Support
==================

GPG is supported for incoming messages at the moment:

* Working
    * You can validate signed messages.
    * You can decrypt outgoing messages.

* Not Working, or implemented.
    * You cannot sign outgoing messages.
    * You cannot encrypt outgoing messages.



Configuration
-------------

There is no configuration required, provided you have the `mimegpg`
binary installed upon your system.

If `mimegpg` is detected then incoming messages which are signed
will be verified automatically.
