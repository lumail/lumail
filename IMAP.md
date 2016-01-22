Lumail IMAP Support
===================


Required IMAP Operations
------------------------

In brief we need the following operations to work to support IMAP
access of mail, in a useful fashion:

* Login and get the list of folders.
* Retrieve the messages from a given folder.
* Get/Set the flags of a given message.
* Delete a message.
* Save a message.

Looking around there are no simple, portable, and sane C++ libraries
for IMAP-client operations which lead me to avoid IMAP for quite some
time.


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

Because I didn't want to write my own IMAP library, or spend further
time trying to update the curl-based code to work in a faster and
more efficient fashion I took a step back.

The specific goal is that we can do "stuff" over IMAP from our C++
core - because this is where mail-folders are examined and messages
retrieved - it crossed my mind that we could leverage the reliable
Perl [Net::IMAP::Client](http://search.cpan.org/perldoc?Net%3A%3AIMAP%3A%3AClient) module, by writing a couple of helpers.

With that in mind I've put together a simple server that creates
a Unix domain-socket, reads command from it, and sends the response
back as JSON-encoded objects.

All of our C++ code which needs to make IMAP operations uses the
domain-socket, sending requests over it, and reading back the reponses.


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

