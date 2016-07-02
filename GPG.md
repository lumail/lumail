Lumail GPG Support
==================

Lumail now contains (experimental) support for GPG, which covers:

* Verifying signatures.
* Decrypting messages.
* Encryting outgoing email.
* Signing outgoing email.

All of this GPG-support is implemented via the use of the `mimegpg` tool, which is part of the courier mail-server.  Upon a Debian GNU/Linux system the binary can be found by installing the sqwebmail package:

    # apt-get install sqwebmail


Configuring & Testing mimegpg
-----------------------------

mimegpg doesn't have any configuration, but it will assume that you're using a gpg-agent.  So you'll need to ensure that you have an agent setup, and that your GPG-key has its passphrase unlocked.

Configuring this is outside the scope of this documentation.

To test that things are working you'll want to do two things:

* Sign a message.
* Verify that signature.

If both these things work as expected you should find that lumail's integrated support "just works".

To test signing a message please run:

    $ mimegpg -s -- --batch --trust-model always  < /etc/passwd

If all is OK you should find:

* You are _not_ prompted for a GPG-passphrase.
* You receive output that ends with a signature.

A signature will look like this:

     -----BEGIN PGP SIGNATURE-----
     Version: GnuPG v1
     ...
     ..
     -----END PGP SIGNATURE-----


If that works then you can move on to verifying a signature, which involves running the same command and then piping the output to a second mimegpg command to run the verification:

    $ mimegpg -s -- --batch --trust-model always  < /etc/passwd  | mimegpg -d -c  | grep gpg:
    gpg: Signature made Sat 02 Jul 2016 07:11:08 AM EEST using RSA key ID 0C626242
    gpg: Good signature from "Steve Kemp (Edinburgh, Scotland) <steve@steve.org.uk>"
    gpg: WARNING: This key is not certified with a trusted signature!
    gpg:          There is no indication that the signature belongs to the owner.

If you receive a "Good signature" output, then all is OK.


Incoming Email
--------------

GPG is supported for incoming messages at the moment:

* You can validate signed messages.
* You can decrypt outgoing messages.

Of course to verify a message you will need the GPG-key of the sending user upon your local keyring.  If it isn't present then the verification will be attempted but will fail.


Outgoing Email
--------------

You can choose to sign your outgoing messages, encrypt your outgoing messages, or both.

In the case of encryption the recipient's email will be used to determine the key to use.

To enable this just press `g` when prompted and then make your selection.
