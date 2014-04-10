----
----
--
-- When lumail launches it will look for the following configuration files:
--
--   /etc/lumail.lua
--   /etc/lumail.d/*.lua
--   ~/.lumail/config.lua
--
-- Each of these files will be loaded in turn.  If none are present then
-- the client will abort with an error message.
--
-- If you wish to load additional configuration file(s) you may do so via
-- the "--rcfile" argument.  Multiple additional files may be specified:
--
--   $ lumail --rcfile ./one.lua --rcfile ./two.lua
--
-- NOTE: This file will also cause ~/.lumail.d/*.lua to be loaded.
--
---
---
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
-- The lumail primitives themselves, from which the examples are built, are
-- documented here:
--
--   http://lumail.org/lua/
--
----
----
--






--
-- This setting is used to specify the prefix for your maildir hierarchy.
--
-- (The directory specified will be processed recursively.)
--
prefix = os.getenv( "HOME" ) .. "/Maildir"

--
-- Ensure the directory exists.
--
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
-- Set the from address for yourself, which is used for composing/replying.
--
-- This can be changed in one of the available hooks.
--
-- In this configuration file we use the `on_folder_selection()` function
-- to change this on a per-folder basis.
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
--   "pat" -> Show all maildir folders which match the regular expression "pat".
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
--       "pat" -> Show all messages which match the regular expression "pat".
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
headers = { "$TO", "$CC", "$FROM", "$DATE", "$SUBJECT" }


--
-- Choose the highlight mode for the selected column in maildir and index mode.
-- Valid options are:
--
--   underline,standout,reverse,blink,dim,bold.
--
-- These settings might be useful if you run inside a terminal multiplexer such as tmux.
--
-- NOTE: You can use multiple values here:
--
index_highlight_mode('bold standout')
maildir_highlight_mode('bold standout')



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
-- It is called via the on_idle() function defined later.
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
-- This function is called when a folder is added/removed/toggled
-- within the selected set.
--
-- This function is a perfect place to run per-folder hooks.
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

   msg( "There are " .. count_messages() .. " messages present." )
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
      --
      -- TODO: Fixe this for messages.
      --

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
      --
      -- TODO: Fix this for message/text
      --

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
function on_read_message( path )
   if ( is_new()  ) then
      mark_read()
   end
end



--
-- This is called if the user replies "no" to the "send message?"
-- prompt which is issued at the end of the compose() and reply()
-- functions.
--
function on_message_aborted( path )
   msg( "Sending message aborted." )
end


--
-- This function is called when a message is deleted.
--
-- You might save the message to ~/Maildir/.trash, or similar.
--
function on_delete_message( path )
   msg( "Message deleted: " .. path )
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
   -- Add the colours we support for the various display functions,
   -- as well as the highlight options.
   --
   ret = {
      "blue", "cyan", "green", "magenta", "red", "white", "yellow",
      "blink", "bold", "dim", "reverse", "standout", "underline"
   }

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
-- This function will be called when the global mode changes
--
-- Here we define a closure so that the previous mode can be
-- returned to easily.
--
-- (In our case this is so that when viewing a file you can revert
-- to the previous mode.  See the `display_file` primitive and the
-- function named 'show_file` later in this file.)
--
do

   -- We store the previous global-mode here, when changing the mode.
   local pmode = ''

   function on_mode_change( old, new )
      -- check if we are switching from maildir to index mode, if so try to jump
      -- the first unread mail or go to the last one
      if string.find(old,'maildir') and string.find(new,'index') then
         if (newmail_displayed()) then
            jump_to_first_unread()
         else
            jump_to_end()
         end
      end
      pmode = old
   end

   function previous_mode()
      global_mode( pmode )
   end
end


--
-- Test if there is currently a new message displayed
--
function newmail_displayed()

   --
   -- save old limit + mode
   --
   -- See also: http://lumail.org/examples/state.html
   --
   old_index_limit = index_limit()
   old_global_mode = global_mode()

   -- check if we can count any new messages
   index_limit( "new" )
   global_mode( "index" )
   c = count_messages()

   -- restore
   index_limit( old_index_limit )
   global_mode( old_global_mode )

   if ( c > 0 ) then
      return true
   else
      return false
   end
end


--
-- Jump to the first unread mail starting from pos.
--
-- Do nothing if all mails are marked read
--
function jump_to_next_unread_from_pos(pos)
   cur = index_offset()
   count = count_messages()
   i = pos
   while( i < count ) do
      jump_index_to( i )
      if is_new() then
         break
      end
      i = i + 1
   end
   if i == count then
      jump_index_to(cur)
   end
end


--
-- This function will jump to the first unread message. If all messages are read
-- to nothing
--
function jump_to_first_unread()
   jump_to_next_unread_from_pos(0)
end


--
-- This  function will jump to the next unread message.
-- If all are read do nothing
--
function jump_to_next_unread()
   cur = index_offset()
   jump_to_next_unread_from_pos(cur+1)
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
-- Show a file.
--
function show_file( filename )
   show_file_contents(filename)
   global_mode( "text" )
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
-- When in maildir-mode show all folders which have a path matching
-- the given regular expression.
--
function livejournal_folders()
   maildir_limit( "livejournal.[0-9]" )
   clear()
end


--
-- Navigation function:  Scroll "something" down.
--
function down()
   m = global_mode()

   if ( string.find( m, "maildir" ) ) then
      scroll_maildir_down( 1 )
   elseif (string.find(m, "index" ) ) then
      scroll_index_down( 1 )
   elseif (string.find(m, 'message') ) then
      scroll_message_down( 1 )
   elseif (string.find(m, 'text') ) then
      scroll_text_down( 1 )
   else
      msg( "down() not implemented for mode:" .. m )
   end
end

--
-- Navigation function:  Scroll "something" up.
--
function up()
   m = global_mode()

   if ( string.find( m, "maildir" ) ) then
      scroll_maildir_up( 1 )
   elseif (string.find(m, "index" ) ) then
      scroll_index_up( 1 )
   elseif (string.find(m, 'message') ) then
      scroll_message_up( 1 )
   elseif (string.find(m, 'text') ) then
      scroll_text_up( 1 )
   else
      msg( "up() not implemented for mode:" .. m )
   end
end


--
-- Navigation function:  Scroll "something" down a page.
--
function page_down()
   m = global_mode()

   if ( string.find( m, "maildir" ) ) then
      -- The minus-two is to account for the status-area
      scroll_maildir_down( screen_height() - 2  )
   elseif (string.find(m, "index" ) ) then
      -- The minus-two is to account for the status-area
      scroll_index_down( screen_height() - 2  )
   elseif (string.find(m, 'message') ) then
      -- The minus-two is to account for the status-area
      scroll_message_down( screen_height() - 2  )
   elseif (string.find(m, 'text') ) then
      -- The minus-two is to account for the status-area
      scroll_text_down( screen_height() - 2  )
   else
      msg( "page_down() not implemented for mode:" .. m )
   end
end


--
-- Navigation function:  Scroll "something" up a page.
--
function page_up()
   m = global_mode()

   if ( string.find( m, "maildir" ) ) then
      -- The minus-two is to account for the status-area
      scroll_maildir_up( screen_height() - 2  )
   elseif (string.find(m, "index" ) ) then
      -- The minus-two is to account for the status-area
      scroll_index_up( screen_height() - 2  )
   elseif (string.find(m, 'message') ) then
      -- The minus-two is to account for the status-area
      scroll_message_up( screen_height() - 2  )
   elseif (string.find(m, 'text') ) then
      -- The minus-two is to account for the status-area
      scroll_text_up( screen_height() - 2  )
   else
      msg( "page_up() not implemented for mode:" .. m )
   end
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





do
   --
   -- This variable is used to allow searches to reuse prior input.
   --
   local search_next_prev = ''

   --
   -- Search for the next folder/message which matches the entered pattern.
   --
   function search_next()

      --
      -- The search pattern.
      --
      x = prompt("/:" )

      --
      -- If the user didn't enter anything then use the previous input.
      --
      if ( x == "" ) then x = search_next_prev end

      --
      -- Save the input for the next search.
      --
      search_next_prev = x

      --
      --  Get the mode, so we can do the correct kind of searching.
      --
      m = global_mode()

      if ( string.find( m, "maildir" ) ) then
         scroll_maildir_to( x )
      elseif (string.find(m, "index" ) ) then
         scroll_index_to( x )
      else
         msg( "search_next() not implemented for mode:" .. m )
      end
   end
end



--
-- Open a folder exclusively.
--
-- If called with no arguments it will open the folder under the
-- cursor in maildir-mode.
--
-- If called with the name of a folder it will open that specified
-- folder.
--
function open_folder(folder)
   clear_selected_folders()

   if ( folder ) then
      set_selected_folder(folder)
   else
      add_selected_folder()
   end
   index()
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
-- If the function "on_reply_transform_subject()" is defined it will
-- be invoked when a message is replied to.
--
-- The intention is that you return an updated subject.
--
function on_reply_transform_subject( sub )

   if ( string.len( sub ) < 1 ) then
      return "No subject"
   end
   --
   -- Remove (repeated) "Re:" from the start of string.
   --
   while( string.find(sub, "^[rR][eE]:" ) ) do
      sub = string.gsub( sub, "^[rR][eE]:[ \t]+", "" )
   end

   -- Add prefix
   return "Re: " .. sub
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
keymap['text']    = {}

--
-- Global Bindings
--
-- Quit
keymap['global']['Q'] = "exit()"
-- Help
keymap['global']['H'] = "help()"
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
keymap['global']['^R'] = 'clear()'
keymap['global']['^L'] = 'clear()'
keymap['global']['KEY_RESIZE'] = 'clear()'

-- Scroll up/down
keymap['global']['j'] = 'down()'
keymap['global']['KEY_DOWN'] = 'down()'
keymap['global']['J'] = 'page_down()'
keymap['global']['KEY_NPAGE'] = 'page_down()'

keymap['global']['k'] = 'up()'
keymap['global']['KEY_UP'] = 'up()'
keymap['global']['K'] = 'page_up()'
keymap['global']['KEY_PPAGE'] = 'page_up()'


--
-- THIS WORKS :D.
--
--
keymap['global']['Ѫ'] = 'msg("UTF-8 rocks!")'

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
-- Find folders
--
keymap['maildir']['/'] = 'search_next()'
keymap['maildir']['f'] = 'faves()'


--
-- Search the message index
--
keymap['index']['/'] = 'search_next()'

--
-- Per-message actions
--
keymap['index']['Space'] = 'view_message()'
keymap['index']['Enter'] = 'view_message()'
keymap['index']['r'] = 'reply()'
keymap['index']['f'] = 'forward()'
keymap['index']['d'] = 'delete()'

--
-- View all/new messages only.
--
keymap['index']['a'] = 'index_limit("all")'
keymap['index']['n'] = 'index_limit("new")'

--
-- Jump to the next unread mail in index or message mode
--
keymap['index']['N']   = 'jump_to_next_unread()'
keymap['message']['N'] = 'jump_to_next_unread()'

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

-- wrap linese
keymap['message']['w'] = 'toggle_wrap_lines()'

-- Actions
keymap['message']['d'] = 'delete()'
keymap['message']['f'] = 'forward()'
keymap['message']['r'] = 'reply()'


--
-- Text display
--
keymap['text']['q'] = 'previous_mode()'


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


--
-- Jump between showing the full-path to all Maildirs
-- and just the short-version.
--
function toggle_maildir_names()

   -- get the current format-string.
   fmt = maildir_format()

   -- if it contains the short name
   if ( fmt:find( "$NAME", 1, true ) ) then
      -- replace it with the complete path.
      fmt = fmt:gsub( "$NAME", "$PATH" )
   else
      -- otherwise if it contains the path
      if ( fmt:find( "$PATH", 1, true ) ) then
         -- replace with the short-name.
         fmt = fmt:gsub( "$PATH", "$NAME" )
      end
   end

   --
   -- Update the format.
   --
   maildir_format( fmt )
end

--
-- Bind to a key.
--
keymap['global']['p'] = "toggle_maildir_names()"



--
--  Test the show_text() primitive.
--
function x()
   txt = { "${colour:red}Steve",
           "${colour:blue}Kemp",
           "${bold}Has",
           "${underline}Definitely",
           "Made this work" }
   show_text( txt )
   global_mode( "text")
end