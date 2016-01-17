

Required IMAP Operations
------------------------

In brief we need the following operations to work to support IMAP
access of mail, in a useful fashion:

* Login and get the list of folders.
* Retrieve the messages from a given folder.
* Get/Set the flags of a given message.

Looking around there are no simple, portable, and sane C++ libraries
for IMAP-client operations.


IMAP Implementation
-------------------

The initial attempt at adding support for IMAP used the `libcurl`
library.  This worked but was awfully slow, largely because of the
number of round-trips in the naive implementation.

For example to get the list of folders required one trip to get all
the folder names, then for each folder two more trips:

* One to count the total-messages.
* One to count the unread-messages.

With 500 folders this would equate to 1001 network trips, each time
potentially being slowed by the need to negotiate SSL afresh.

Given that I have zero desire to write my own IMAP library, a simple
wrapper had to be the option, but the available libraries looked
either horribly complex, or painfully naive.

On that basis the choice seems to be to use somebody elses IMAP
library - something that could be bound to the C++ implementation
of Lumail.

Enter Perl.


IMAP via Perl
-------------

As a proof of concept I've written a pair of scripts, in the top-level
perl.d/ directory.  These each read the IMAP login credentials, and
target-server, via the environment, and perform the necessary magic.

In the case of listing folder *ONE* network request is sufficient
to retrieve:

* The list of available folders.
* The total/unread count of each folder.

This is a collosal win.

To open a remote folder we also make one request which will receive
the "flags" and "bodies" of each message in the folder.  This is
returned as a JSON-array which the C++ code parses via the use
of [jsoncpp](https://github.com/jmhodges/jsonpp).




Setup Instructions
------------------

Ensure you have the required dependencies:

     apt-get install libnet-imap-client libjson-perl

Configure your lumail with suitable IMAP settings:

     Config:set( "imap.cache", "/tmp" )
     Config:set( "imap.server",   "imaps://imap.gmail.com/" )
     Config:set( "imap.username", "steve.login.name" )
     Config:set( "imap.password", "pass.word" )

Enjoy.


TODO
----

* Document this in the README.md.
* Handle flags correctly.
    * Add a script for "get/set" flags.
* Handle saving to Sent-mail.
    * Probably create a script which can be called via this lua:
    * "imap:save_message( "/tmp/blah", "Sent-Mail" )"
    * Or similar.
* Handle deleting specially for IMAP folders too.
