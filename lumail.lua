--
-- When lumail launches it will look for two configuration files, if they
-- exist they will be loaded in turn:
--
--   /etc/lumail.lua
--   ~/.lumail/config.lua
--
-- If you wish to load an additional configuration file you can add it via
-- the --rcfile argument:
--
--   $ lumail --rcfile ./lumail.lua
--
--
-- NOTE: This file will also ensure that each file matching *.lua
--       beneath ~/.lumail.d/ will be loaded.
--
-- The configuration file(s) are responsible for determining which keystrokes
-- are recognized, and what they will do.
--
-- The configuration file(s) can contain functions you define yourself, using
-- the Lua primitives that lumail implements internally.  You can see several
-- examples online here:
--
--   http://lumail.org/examples/
--
-- The lumail primitives themselves are documented here:
--
--   http://lumail.org/lua/
--
-- Steve
-- --
--






--
-- This setting is used to specify the prefix for your maildir hierarchy.
--
-- (The directory specified will be processed recursively.)
--
prefix = os.getenv( "HOME" ) .. "/Maildir"

if ( is_directory(prefix) ) then
   maildir_prefix(prefix)
else
   abort( "The specified Maildir prefix isn't a directory: " .. prefix )
end


--
-- There is only one folder which is special, and that is the one where
-- lumail will record copies of outgoing mail(s).
--
-- The sent-mail folder may be specified here, if it exists:
--
-- NOTE:  This is commented out by default.
--
-- if ( is_maildir( maildir_prefix() .. "/sent-mail" ) ) then
--   sent_mail( maildir_prefix() .. "/sent-mail" )
--end


--
-- Set the from address for yourself, which is used for
-- composing/replying.
--
-- This can be changed in one of the available hooks.
--
-- In this configuration file we use the `on_folder_selection()` function later.
--
default_email = "Steve Kemp <steve@steve.org.uk>"
from( default_email )


--
-- Specify the outgoing mail binary.
--
-- Prefer /usr/sbin/sendmail, but use /usr/lib/sendmail if that exists
--
if ( executable( "/usr/sbin/sendmail" ) ) then
   sendmail_path( "/usr/sbin/sendmail -t" )
end
if ( executable( "/usr/lib/sendmail" ) ) then
   sendmail_path( "/usr/lib/sendmail -t" )
end

--
-- Specify the editor to use for composing/replying to email messages.
--
-- If this isn't set lumail will default to using the value of the
-- environmental variable "EDITOR", and if that is unset it will use
-- "vim" with no default PATH specified.
--
--
-- Here we try to use "vi", but will use "vim" in preference if both
-- are present.
--
if ( executable( "/usr/bin/vi" ) ) then
   editor( "/usr/bin/vi" )
end
if ( executable( "/usr/bin/vim" ) ) then
   editor( "/usr/bin/vim" )
end


--
-- Show all folders by default
--
-- When called with no arguments this function will return the current
-- setting, otherwise it will update it.  This is a common pattern for lumail,
-- each of the functions which can get or set a value works in the same way.
--
-- The maildir limit can be set to three values:
--
--    all  -> Show all maildir folders.
--    new  -> Show all maildir folders which contain unread messages.
--   "str" -> Show all maildir folders which match the substring "str".
--
-- See "all_folders" , "new_folders", and "livejournal_folders" functions for
-- example of use.
--
maildir_limit( "all" )


--
-- The maildir format controls how maildirs are displayed in maildir-mode.
--
-- Valid options are currently limited to:
--
--   $CHECK  - "[x]" if the folder is selected, "[ ]" otherwise.
--   $TOTAL  - The total number of messages in the maildir.
--   $NEW    - The number of unread/new messages in the maildir.
--   $UNREAD - The number of unread/new messages in the maildir.
--   $READ   - The number of read/old messages in the maildir.
--   $PATH   - The maildir path.
--   $NAME   - The maildir name (i.e. dirname($PATH)).
--
maildir_format( "$CHECK - $UNREAD/$TOTAL - $PATH" )


--
-- When opening folders we'll show all messages.
--
-- Valid options are:
--
--        all  -> Show all message.
--        new  -> Show all unread messages.
--       "str" -> Show all messages which match the substring "str".
--
index_limit( "all" )


--
-- The index format controls how messages are displayed inside folder lists.
--
-- Valid options are currently limited to:
--
--   $DAY   (day of week)
--   $MONTH (month name)
--   $MON   (month abbreviation)
--   $YEAR  (year)
--   $DATE
--   $FLAGS
--   $FROM
--   $SUBJECT
--   $TO
--
--
index_format( "[$FLAGS] $DAY/$MONTH/$YEAR $FROM - $SUBJECT" )


--
-- The headers we show when displaying a mail.
--
-- (The headers listed are shown in the order they are specified here.)
--
headers = { "$TO", "$FROM", "$DATE", "$SUBJECT" }


--
-- When viewing the contents of a message it is possible to pipe
-- the body through a filter.  Filters can be useful for trimming
-- excessive quotes, etc.
--
-- The utility "t-prot" is a well-known example of a filter that
-- can make an email nicer to view, and it can be used with a
-- setting such as this one:
--
--     display_filter( "t-prot -cmekatlS --bigq --pgp-move-vrf -Mmutt -L/etc/t-prot/footers -A/etc/t-prot/ads" )
--
-- The following would convert each message to be 100% upper-case
--
--     display_filter( "tr '[:lower:]' '[:upper:]'" )
--
-- Note: There is no filter by default.
--


--
-- This function will invoke offlineimap, if it is installed
-- and configured for the current user.
--
function offlineimap()
   if ( not file_exists( os.getenv( "HOME" ) .. "/.offlineimaprc" ) ) then
      return false
   end
   if ( not executable( "/usr/bin/offlineimap" ) ) then
      return false
   end

   exec( "/usr/bin/offlineimap" )
   clear()
   return true
end

--
-- This function is called when the client is launched.
--
-- You might consider something useful like this:
--
--    function on_start()
--       offlineimap()
--    end
--
function on_start()
   msg("lumail v" .. VERSION .. " http://lumail.org/" )
end


--
-- This function is called when a folder is added/removed to the selected set.  Or toggled.
--
-- It is where you can run per-folder hooks.
--
-- Remember: More than one folder might be selected at any given time.
--
function on_folder_selection( folder )

   --
   -- Change the email address we use depending on our folder.
   --
   if ( string.find( folder, "debian-administration", 1, true )  ) then
      from ("Steve <steve@debian-administration.org>")
   elseif ( string.find( folder, "debian" , 1, true )  ) then
      from("Steve <skx@debian.org>")
   else
      from( default_email )
   end

   msg( "There are " .. count_messages() .. " messages" )
end


--
-- Jump to the end of the screen, regardless of mode.
--
function jump_to_end()

   mode = global_mode()
   mode = string.lower( mode )

   if ( string.find( mode, "maildir" ) ) then

      -- Jump to last Maildir in the list.
      jump_maildir_to( count_maildirs() - 1 )

   elseif (string.find(mode, "index" ) ) then

      -- Jump to the last message in the list.
      jump_index_to( count_messages() - 1 )

   else

      -- Jump to the end of the message.
      scroll_message_to( count_lines() - 2 )
   end
end


--
-- Jump to the start of the screen, regardless of mode.
--
function jump_to_start()

   mode = global_mode()
   mode = string.lower( mode )

   if ( string.find( mode, "maildir" ) ) then

      -- Jump to the first Maildir
      jump_maildir_to( 0 )

   elseif (string.find(mode, "index" ) ) then

      -- Jump to the first message.
      jump_index_to( 0 )

   else

      -- Jump to the start of the message.
      scroll_message_to( 0 )
   end
end


--
-- This function is called when a message is displayed.
--
-- The argument is the path to the message on-disk.
--
-- Here we use the "is_new()" and "mark_read()" functions with that path.
--
-- (If the path were omitted then the current-message would be implicit.)
--
-- We mark any new message(s) read when we read them here.
--
function on_read_message( path )
   if ( is_new()  ) then
      mark_read()
   end
end


--
-- Show the version of this client.
--
function show_version()
   msg("This is lumail version " .. VERSION .. " from http://lumail.org/" )
end




--
-- This function is called at run-time when TAB-completion is invoked.
--
-- The function is expected to return a Lua table, the values of which
-- will be used for completion.  (The keys being ignored.)
--
-- Here we add the known colours for the various colour-related primitives,
-- as well as any user-defined function.
--
-- NOTE: User-functions come from the global Lua "_G" table.
--
--
function on_complete()

   --
   -- Add the colours we support for the various display functions.
   --
   ret = { "blue", "cyan", "green", "magenta", "red", "white", "yellow" }

   --
   -- Add in all user-defined functions.
   --
   for k,v in pairs(_G) do
      ret[k] = k
   end

   return(ret)
end


--
-- This function is called when the client exits.
--
-- It could also be used to run a final imap-sync, or similar.
--
function on_exit()
   print("Lumail has now terminated.")
   io.write( "That's it man, game over man, game over!\n")
end


--
-- The 'on_idle()' function is called approximately once a second,
-- when the client is idle.
--
-- Here we wrap that function up a little such that we can regularly
-- shell out to sync our mail from the remote location it is delivered.
--
-- Every second we also update the status-area to show a message.
--
do

   -- Last sync time, in seconds-past-epoch.
   local ls = os.time()

   function on_idle()
      m = global_mode()
      m = string.lower( m )

      str = ""

      -- Set the message we'll display
      if ( string.find( m, "maildir" ) ) then
         str = "mode:" .. m ..  " limit:" .. maildir_limit()
      else
         str = "mode:" .. m
      end

      -- Show the message & the time.
      msg( str .. " time:" .. os.date("%X" ) )

      -- Dump the internal Lua-stack for debug-purposes.
      dump_stack()

      --
      -- If the time between the last sync is more than
      -- five minutes resync mail.
      --
      ct = os.time()
      if ( ( ct - ls ) >=  ( 60 * 5 ) ) then
         ls = ct
         if ( offlineimap() ) then
            msg( "offlineimap has synced your mail" )
         else
            msg("offlineimap not available." )
         end
      end
   end
end


--
-- Switch to the index-view mode.
--
-- This will display the messages contained in all currently-selected folders.
--
-- (Selected folders will be displayed with a "[x]" next to them in maildir-mode.)
--
function index()
   global_mode( "index" )
   clear()
end


--
-- Switch to the maildir-mode.
--
function maildir()
   global_mode( "maildir" )
   clear()
end


--
-- Move to message-mode with the currently selected message.
--
function view_message()
   global_mode( "message")
   clear()
end


--
-- When in maildir-mode show all folders.
--
function all_folders()
   maildir_limit( "all" )
   clear()
end


--
-- When in maildir-mode show all folders which contain unread messages.
--
function new_folders()
   maildir_limit( "new" )
   clear()
end


--
-- When in maildir-mode show all folders which have a path matching the given pattern.
--
function livejournal_folders()
   maildir_limit( "livejournal.[0-9]" )
   clear()
end


--
-- Navigation functions for maildir-mode
--
function maildir_down()
   scroll_maildir_down( 1 )
end
function maildir_page_down()
   -- The minus-two is to account for the status-area
   scroll_maildir_down( screen_height() - 2  )
end
function maildir_up()
   scroll_maildir_up(1)
end
function maildir_page_up()
   -- The minus-two is to account for the status-area
   scroll_maildir_up( screen_height() -2 )
end



--
-- Navigation functions for index-mode
--
function message_down()
   scroll_index_down( 1 )
end
function message_page_down()
   -- The minus-two is to account for the status-area
   scroll_index_down( screen_height() - 2 )
end
function message_up()
   scroll_index_up(1)
end
function message_page_up()
   -- The minus-two is to account for the status-area
   scroll_index_up( screen_height() - 2 )
end


--
-- Should we wrap lines when showing a message?
--
function toggle_wrap_lines ()
    if wrap_lines == nil then
        wrap_lines = true
    elseif wrap_lines == true then
        wrap_lines = false
    elseif wrap_lines == false then
        wrap_lines = true
    end
end



--
-- Search for the next folder/message which matches the entered pattern.
--
function search_next()
   x = prompt("/:" )
   m = global_mode()

   if ( string.find( m, "maildir" ) ) then
      scroll_maildir_to( x )
   elseif (string.find(m, "index" ) ) then
      scroll_index_to( x )
   else
      msg( "search_next() not implemented for mode:" .. m )
   end
end



--
-- Open a folder exclusively.
--
-- This behaves like the traditional "open folder" function in our favourite
-- client.  I like to open multiple folders at once, because that's how I roll.
--
function open_folder()
   clear_selected_folders()
   add_selected_folder()
   global_mode( "index" )
   clear()
end



--
-- Open my most important folders.
--
function faves()
   global_mode( "index" )

   -- ensure all folders are available
   maildir_limit( "all" )

   -- clear the currently selected folders.
   clear_selected_folders()

   -- The folders we care about
   interesting = { "/.steve.org.uk", "/.people.kirsi",  "/.edinburgh-portraits.com" }

   -- For each one we're going to open
   for index,name in ipairs( interesting ) do

      -- Jump to first folder.
      jump_maildir_to( 0 )

      -- Scroll forward until we find the matching folder
      scroll_maildir_to( name )

      --
      -- Note: "current_maildir" will return the value we've found.
      --
      -- We could check if the folder returned matches that which
      -- we've found here.  In my case I don't need to.
      --

      -- Add it to the selected set.
      add_selected_folder()

   end

   -- Now we've selected our folders, open them.
   global_mode( "index" )
   clear()
end


--
-- Mark all messages in the current folder as read.
--
function mark_all_read()
   count = count_messages()
   i = 0
   while( i < count ) do
      jump_index_to( i )
      mark_read()
      i = i + 1
   end
end


--
-- Return contents of the given file.
--
-- Note: Called by get_signature()
--
function file_contents(file)
    local f = io.open(file, "rb")
    if ( f ) then
       local content = f:read("*all")
       f:close()
       return( content )
    else
       return( "Error reading file: \"" .. file .. "\"" )
    end
end


--
-- This function is called when you compose an email, the return value
-- is the signature text to insert at the end of the new mail.
--
-- In this example we try to lookup a signature based on the domains of
-- the sender and the recipient.  We stop at the first match.
--
-- If neither of those succeed we return the content of ~/.signature,
-- if it is present.
--
-- For example:
--
--   Given a sender of "steve@steve.org.uk", and recipient of bob@example.com
--
--   1. We'd read and return the contents of  ~/.sigs/example.com if present.
--
--   2. If not we'd read and return the contents of  ~/.sigs/steve.org.uk if present.
--
--   3. If not we'd read and return the contents of ~/.signature
--
--   4. We give up and return "".
--
function get_signature( from, to, subject )

   --
   -- Get the home directory, if this fails we'll return
   -- an empty string.
   --
   home = os.getenv( "HOME" )
   if ( not home ) then
      return ""
   end

   --
   -- The two addresses we'll try to process.
   --
   addresses = {}
   addresses[1] = to
   addresses[2] = from

   for offset,addr in pairs(addresses) do

      -- strip anything except the address, by looking between: <>
      -- i.e. "Steve Kemp" <steve@example.com> becomes steve@example.com
      email = string.match(addr, "<(.*)>" ) or addr

      -- get the domain, lowercase it.
      domain = string.sub( email, string.find( email, "@" )+1 )
      domain = string.lower(domain)

      -- Look for the domain-file beneath ~/.sigs/
      file = home .. "/.sigs/" .. domain
      if ( file_exists( file ) ) then
         return( file_contents( file ) )
      end
   end

   --
   -- Fallback to ~/.signature if present
   --
   if ( file_exists( home .. "/.signature" ) ) then
      return( file_contents( home .. "/.signature" ) )
   else
      --
      -- No signature found.  Return empty text.
      --
      return ""
   end

end



--
-- Now setup keymaps for the different modes.
--
-- The same keypress might do different things in different modes,
-- which is one of the advantages of a modal-client.
--
-- However to avoid duplication there is a global keymap.
--
--   For example this works in all modes:
--      kemymap['global']['Q'] = 'exit()'
--
keymap = {}
keymap['global']  = {}
keymap['index']   = {}
keymap['message'] = {}
keymap['maildir'] = {}


--
-- Global Bindings
--
-- Quit
keymap['global']['Q'] = "exit()"
-- Compose a new message.
keymap['global']['c'] = "compose()"
-- repl-mode
keymap['global'][':'] = "loadstring(prompt(\":\"))()"
-- Execute-shell
keymap['global']['!'] = "exec(prompt(\"Shell command:\"))"
-- Move to Maildir-mode
keymap['global']['M']   = 'maildir()'
-- Move to Index-mode
keymap['global']['I'] = 'index()'
-- Show the client version
keymap['global']['v'] = 'show_version()'
-- Jump to end
keymap['global']['*']        = 'jump_to_end()'
keymap['global']['KEY_END']  = 'jump_to_end()'
keymap['global']['KEY_HOME'] = 'jump_to_start()'
-- Refresh display
keymap['global']['^R'] = 'refresh_display()'
keymap['global']['^L'] = 'refresh_display()'
keymap['global']['KEY_RESIZE'] = 'refresh_display()'


--
-- Quitting means different things in different modes.
--
--  If viewing a message quit means return to the index.
keymap['message']['q'] = "index()"
--  If viewing a folder quit means return to the maildir list.
keymap['index']['q'] = "maildir()"
-- If viewing a maildir list then q means exit.
keymap['maildir']['q'] = "exit()"


--
-- Toggle display of folders
--
keymap['maildir']['a'] = 'all_folders()'
keymap['maildir']['n'] = 'new_folders()'
keymap['maildir']['l'] = 'livejournal_folders()'
keymap['maildir']['g'] = 'maildir_limit( "(Google.*INBOX|Google.*All Mail)" )'


--
-- Scroll up/down & find folders
--
keymap['maildir']['j'] = 'maildir_down()'
keymap['maildir']['KEY_DOWN'] = 'maildir_down()'
keymap['maildir']['J'] = 'maildir_page_down()'
keymap['maildir']['KEY_NPAGE'] = 'maildir_page_down()'
keymap['maildir']['k'] = 'maildir_up()'
keymap['maildir']['KEY_UP'] = 'maildir_up()'
keymap['maildir']['K'] = 'maildir_page_up()'
keymap['maildir']['KEY_PPAGE'] = 'maildir_page_up()'
keymap['maildir']['/'] = 'search_next()'
keymap['maildir']['f'] = 'faves()'


--
-- Scroll up/down the message index
--
keymap['index']['j'] = 'message_down()'
keymap['index']['KEY_DOWN'] = 'message_down()'
keymap['index']['J'] = 'message_page_down()'
keymap['index']['KEY_NPAGE'] = 'message_page_down()'
keymap['index']['k'] = 'message_up()'
keymap['index']['KEY_UP'] = 'message_up()'
keymap['index']['K'] = 'message_page_up()'
keymap['index']['KEY_PPAGE'] = 'message_page_up()'
keymap['index']['/'] = 'search_next()'

keymap['index']['Space'] = 'view_message()'
keymap['index']['Enter'] = 'view_message()'
keymap['index']['r'] = 'reply()'
keymap['index']['d'] = 'delete()'

--
-- View all/new messages only.
--
keymap['index']['a'] = 'index_limit("all")'
keymap['index']['n'] = 'index_limit("new")'

--
-- Selection bindings.
--
--  "space" will toggle the current folder as selected.
--  "return" will open the selected folder exclusively.
--
keymap['maildir']['Space'] = 'toggle_selected_folder()'
keymap['maildir']['Enter'] = 'open_folder()'



--
-- Message-handling.
--
--
-- Scroll up/down in the current message.
keymap['message']['j'] = 'scroll_message_down(1)'
keymap['message']['KEY_DOWN'] = 'scroll_message_down(1)'
keymap['message']['k'] = 'scroll_message_up(1)'
keymap['message']['KEY_UP'] = 'scroll_message_up(1)'
keymap['message']['Space'] = 'scroll_message_down(10)'

--
-- scroll to the next/prev message
--
keymap['message']['J'] = 'message_down()'
keymap['message']['K'] = 'message_up()'

-- wrap linese
keymap['message']['w'] = 'toggle_wrap_lines()'

-- Actions
keymap['message']['r'] = 'reply()'
keymap['message']['d'] = 'delete()'



---
--
-- Load any local files.
--
-- This will load ~/.lumail.d/*.lua
--
---

if ( is_directory( os.getenv( "HOME" ) .. "/.lumail.d" ) ) then
   load_directory( os.getenv( "HOME" ) .. "/.lumail.d" )
end



--
-- Store persistant history.
--
history_file( os.getenv( "HOME" ) .. "/.lumail.history" )

---
--
--   Further primitives which are not included here are documented
--  online:
--
--       http://lumail.org/lua/
--
--
--   Larger examples are also available online:
--
--       http://lumail.org/examples/
--
--
--
---
