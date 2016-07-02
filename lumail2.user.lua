--
-- This is the per-user configuration file for lumail.
--
-- To make it active copy it to either:
--
--   ~/.lumail2/lumail2.lua
--   ~/.luamil2/$(hostname --fqdn).lua
--
-- Changes here will override the defaults which are loaded by the
-- global configuration file (/etc/lumail2/lumail2.lua).
--
-- It is *highly* recommended you follow this approach, because this
-- will ensure that upgrades do not trash your edits.
--
-- If there are omissions where per-user configuration makes it impossible
-- to configure things as you wish they are bugs, and should be reported
-- as such.
--
--


--
-- NOTE: If you wish you may re-declare any global configuration functions
--       in this file, or remap keybindings.
--
-- For example I like to use `ctrl-w` to exit the client:
--
keymap['global']['^W'] = 'os.exit(1)'



--
-----------------------------------------------------------------------------

--
-- The following line enables *experimental* conversion of message-bodies
-- to UTF-8.
--
-- You might require this to be enabled if you receive mail in foreign
-- character-sets.  It should be harmless when enabled, but character-sets,
-- encoding, and similar, are scary and hard.  If you see garbled text,
-- unreadable emails, or similar problems with displaying message-bodies
-- comment it out as a first step in isolating the problem.
--
Config:set( "global.iconv", 1 )

--
-- The following line will draw one blank line at the bottom of the screen
-- and is designed to mask any artifacts caused by bogus wrapping and drawing
-- which might become evident.
--
-- It is disabled by default.
--
-- Config:set( "global.over-draw", 1 )
--


--
-- The following flag configures our index-mode to only format
-- messages which are visible.
--
-- This is a huge performance boost if you have thousands of messages
-- in a folder, OR if you're using IMAP.
--
-- The downside is that searching through messages "/" and `?`,
-- will break, as items off-screen won't be formatted.
--
Config:set( "index.fast", 1 )

--
-- Get our home-directory, as this is often used.
--
local HOME = os.getenv("HOME")

--
-- The default Maildir location is ~/Maildir
--
Config:set( "maildir.prefix", HOME .. "/Maildir" );

--
-- NOTE: You could also set an array of prefixes, which will be
-- merged and sorted.
--
-- Config:set( "maildir.prefix", { "/home/steve/Maildir", "/tmp/Maildir" } )
--


--
-- When sending mails a copy of the outgoing message will
-- be archived in the "sent" folder.
--
-- Here we configure the location of that sent-folder.
--
local def_save = HOME .. "/Maildir/sent-mail"
if (Directory:is_maildir(def_save) ) then
   Config:set( "global.sent-mail", def_save)
else
   Panel:append( "WARNING: No sent-mail folder defined!" )
end


--
-- Setup our MTA, the command which actually sends the mail.
--
if ( File:exists( "/usr/lib/sendmail" ) ) then
   Config:set( "global.mailer", "/usr/lib/sendmail -t" )
elseif ( File:exists( "/usr/sbin/sendmail" ) ) then
   Config:set( "global.mailer", "/usr/sbin/sendmail -t" )
else
   Panel:append( "WARNING: No sendmail binary found!" )
end

--
-- Setup our default editor, for compose/reply/forward operations.
--
Config:set( "global.editor", "vim  +/^$ ++1 '+set tw=72'" )

--
-- Setup our default From: address.
--
Config:set( "global.sender", "Some User <steve@example.com>" )

--
-- Unread messages/maildirs are drawn in red.
--
Config:set( "colour.unread", "red" )

--
-- Save persistant history of our input in the named file.
--
Config:set( "global.history", HOME .. "/.lumail2.history" )

--
-- Configure a cache-file, and populate it
--
Config:set( "message.cache", HOME .. "/.lumail2.cache" )
cache:load( Config:get( "message.cache") )

--
-- Set the default sorting method.  Valid choices are:
--
--  `file`   - Use the mtime of the files in the maildir.
--  `date`   - Read the `Date` header of the message(s) - slower than the
--             above method, but works on IMAP too.
--  `subject` - Sort by subject.
--  `from`    - Sort by sender.
--
sorting_method( "file" )

--
--  IMAP setup
--
--  The previous configuration set Lumail to read Maildirs
-- beneath ~/Maildir/
--
--  If you prefer you can load mail from a remote IMAP server
-- to do that set the following values appropriately
--
--
--   -- Setup defaults
--   Config:set( "imap.cache", HOME .. "/.lumail2/imap.cache" )
--   Config:set( "index.sort", "none" )
--   Config:set( "index.fast", "1" )
--
--   -- The proxy-program we're using
--   Config:set( "imap.proxy", "/etc/lumail2/perl.d/imap-proxy" )
--
--   -- Now setup login-details
--   Config:set( "imap.server",   "imaps://imap.gmail.com/" )
--   Config:set( "imap.username", "gmail.username" )
--   Config:set( "imap.password", "pass.word.here" )
--
--  IMAP support is still a little experimental, and for more details
-- you should consult the `IMAP.md` file which comes with lumail2.
--


--
-- Some people like to change email addresses when they change
-- folders.
--
-- Here we allow that.
--
folder_from = {
   ['example.com'] = "steve@example.com",
   ['example.net'] = "steve@example.net",
   ['SPAM']        = "steve@spare.example.net"
}


--
-- Setup our colours - for Maildir-mode
--
colour_table['maildir'] = {
   ['Automated'] = 'yellow|underline',
   ['lists']     = 'green|bold',
}

-- Setup our colours - for index-mode
colour_table['index'] = {
   ['Steve'] = 'blue',
   ['Ian']   = 'blue',
}

-- Setup our colours - for a message
colour_table['message'] = {
   -- headers
   ['^Subject:'] = 'yellow',
   ['^Date:']    = 'yellow',
   ['^From:']    = 'yellow',
   ['^To:']      = 'yellow',
   ['^Cc:']      = 'yellow',
   ['^Sent:']    = 'yellow',

   -- quoting, and nested quoting.
   ['^>%s*>%s*']   = 'green',
   ['^>%s*[^>%s]'] = 'blue',
   ['^>%s$']       = 'blue',
}
