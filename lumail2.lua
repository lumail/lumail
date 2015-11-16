--
-- Lumail2 configuration-file
--
-----------------------------------------------------------------------------
-----------------------------------------------------------------------------
--
-- This is the configuration file for the Lumail 2.x command-line email
-- client.
--
-- The configuration is carried out solely by Lua.
--
-- The client will load two files at startup if they exist:
--
--    /etc/lumail2/lumail2.lua
--
--    ~/.lumail2/lumail2.lua
--
-- If you wish to load additional files please specify them on the
-- command-line:
--
--    lumail2 --load-file /path/to/code.lua
--
-- To load files dynamically, at runtime, you may use the standard
-- Lua facilities to do so, such as `require`.
--
-- For reference to the lumail2 objects please see the API documentation
-- included within the repository as `API.md`, or browsable online at:
--
--    http://lumail.org/api/
--
--
-- Steve
-- --
--
--


--
--  Setup a sane Lua load-path
--
----------------------------------------------------------------------------
-----------------------------------------------------------------------------

--
-- We have bundled at least one LuaRocks library, and so we need to
-- make sure that is loadable.
--
-- We'll configure the load-path to search the following two directories:
--
--    /etc/lumail2/luarocks.d/
--    ~/.lumail2/luarocks.d/
--
package.path = package.path .. ';/etc/lumail2/luarocks.d/?.lua'
package.path = package.path .. ';/' .. os.getenv("HOME") .. '/.lumail2/luarocks.d/?.lua'

--
-- The libraries we've loaded.
--
luarocks_libraries = {}

--
-- The libraries that we want to load
--
luarocks_desired = { "date" }

--
-- Load a library by name.
--
function load_luarocks(name)
   luarocks_libraries[name] = require(name)
end

--
-- Ensure we've loaded our `date` library.
--
for index,name in ipairs(luarocks_desired) do
   pcall( _ENV['load_luarocks'], name)
end




--
-- Setup a cache for objects, and define functions for loading/saving
-- the cache-state.
--
----------------------------------------------------------------------------
-----------------------------------------------------------------------------

local cache = {}


--
-- This holder contains the messages which are currently visible.
--
-- When the user selects a Maildir, or changes the active selection
-- via the `index.limit` setting, then this set of messages will be
-- updated.
--
local global_msgs = {}



--
-- (Re)load our cache, if we have a cache-file defined, and it exists.
--
function cache_load()
   local file = Config:get("message.cache")
   if (file) and File:exists( file ) then
      Panel:append("loading cache from: " .. file )
      for line in io.lines(file) do
         key, val = line:match("([^=]+)=(.*)")
         cache[key] = val
      end
   end
end

--
-- Write out the key/vals from our local cache object to the filename
-- the user has set.  If there is no filename them don't write.
--
function cache_save()
   local file = Config:get("message.cache")
   if (file) then
      local hand = io.open(file,"w")
      for key,val in pairs(cache) do
         hand:write( key .. "=" .. val  .. "\n")
      end
      hand:close()
   end
end

--
-- Flush our (on-disk) cache
--
function cache_flush()
   for k in pairs (cache) do
      cache[k] = nil
   end
   cache_save()
end



--
-- Define some utility functions
--
-----------------------------------------------------------------------------
-----------------------------------------------------------------------------



--
-- String interopolation function, taken from the Lua wiki:
--
--   http://lua-users.org/wiki/StringInterpolation
--
-- Usage:
--
--   print( interp( "Hello $(name)", { name = "World" } )
--
function interp(s, tab)
   return (s:gsub('($%b{})', function(w) return tab[w:sub(3, -2)] or w end))
end


--
-- Helper function to ensure that if anything calls `os.exit`
-- we reset the screen neatly, etc.
--
function os.exit(code)
   Screen:exit()
end


--
-- Split a string on newlines, and return the result as a table.
--
-- This is used in some drawing modes.
--
function string_to_table(str)
   local t = {}
   local function helper(line) table.insert(t, line) return "" end
   helper((str:gsub("(.-)\r?\n", helper)))
   return t
end


--
-- Given a line add the given colour to it - removing any
-- existing colour, uunless that colour is "UNREAD".
--
-- For example an input line such as "Bob" will be made red
-- like so:
--
--      local out = colour_line( "Bob", "red" )
--      print( out )
--
-- The result is:
--
--      $[red]Bob
--
-- If the input was "$[blue]Bob" then the blue would be stripped and
-- updated, but if the input is "$[UNREAD]Bob" it will be left alone.
--
function colour_line( line, colour )
   local c, r = string.match(line, "^%$%[([a-zA-Z]*)%](.*)$" )
   if ( c and r  ) then
      c = c:lower()
      if ( c ~= "unread" ) then
         line = "$[" .. colour .. "]" .. r
      end
      return( line )
   end
   line = "$[" .. colour .. "]" .. line
   return line
end


--
-- Return the contents of the named file.
--
function read_file( path )
   local f = io.open(path, "rb")
   if ( f ) then
      local content = f:read("*all")
      f:close()
      return( content )
   else
      return( "Error reading file: " .. file )
   end
end

--
-- Given a key toggle the setting of that key from
-- true -> false, and vice-versa.
--
function toggle_variable( name )

   local current = Config:get( name )
   if ( current == nil ) then current = 1 end

   if ( current == 1 ) or ( current == "1" )  then
      current = 0
   else
      current = 1
   end

   Config:set( name, current )
end


--
-- Change the mode - and update the panel-title, if we have one.
--
function change_mode( new_mode )
   Config:set( "global.mode", new_mode )
   Panel:append( "Mode changed to:" .. new_mode )
end



--
-- Get the date of the message in terms of seconds past the epoch.
--
-- This is handled by reading the Date: header.
--
-- Returns zero on failure.
--
function Message:to_ctime(m)

   local p = m:path()
   if ( cache["ctime:" .. p] ) then
      return(cache["ctime:" .. p] )
   end

   --
   -- If luarocks library is not present then stat() the message
   -- and handle the sorting that way.
   --
   if ( not luarocks_libraries["date"] ) then
      local stat = File:stat( m:path() )
      local res  = stat['mtime']
      cache["ctime:" .. p] = res
      return(res)
   end

   --
   -- Otherwise the get the Date-header and use that to
   -- get the message-age.
   --
   local d = m:header("Date" )
   if ( d ) then
      local date    = luarocks_libraries["date"]
      local d1      = date(d)
      local seconds = date.diff(d1, date.epoch()):spanseconds()

      cache["ctime:" ..p] = seconds
      return seconds
   end
   return 0
end


--
-- Compare two messages, based upon their date-headers.
--
function compare_by_date(a,b)
   return Message:to_ctime(a) < Message:to_ctime(b)
end

--
-- Compare two messages, based upon from-header.
--
function compare_by_from(a,b)
   return a:header("From") < b:header("From")
end

--
-- Compare two messages, based upon subject-header.
--
function compare_by_subject(a,b)
   return a:header("Subject") < b:header("Subject")
end



--
-- Return our maildirs
--
function maildirs()

   --
   -- Get all maildirs
   --
   local all = Global:maildirs()

   --
   -- Sort them, case-insensitively
   --
   table.sort(all, function (a, b) return a:path():lower() < b:path():lower() end)

   --
   -- Get the maildir.limit
   --
   local limit = Config:get("maildir.limit" )

   --
   -- Temporary copies.
   --
   local ret = {}

   --
   -- All
   --
   if ( limit == nil ) or ( limit == "all" ) then
      for i,o in ipairs(all) do
         table.insert(ret, o)
      end
      Config:set("maildir.max", #ret)
      return ret
   end

   --
   -- New
   --
   if ( limit == "new" ) then
      for i,o in ipairs(all) do
         local unread = o:unread_messages()
         if ( unread > 0 ) then
            table.insert(ret, o)
         end
      end
      Config:set("maildir.max", #ret)
      return ret
   end

   if ( limit == "today" ) then
      local time = os.time()
      local today = time - ( 60 * 60 * 24 )

      for i,o in ipairs(all) do
         -- get modification-time of the maildir
         local mtime = o:mtime()

         -- if it was within the past 24 hours then add it
         if ( mtime > today ) then
            table.insert(ret, o)
         end
      end
      Config:set("maildir.max", #ret)
      return(ret)
   end

   --
   -- Some string to match against.
   --
   if ( limit ) then
      for i,o in ipairs(all) do
         local fmt = Maildir:format(o)
         if ( string.find(fmt, limit) ) then
            table.insert(ret, o)
         end
      end
      Config:set("maildir.max", #ret)
      return(ret)
   end

   --
   -- Not reached
   --
end

--
-- Return our sorted messages
--
function sorted_messages()

   --
   -- If we have a cached selection then we'll return it
   --
   if ( #global_msgs > 0 ) then
      Panel:append( "Returning cached message-set" )
      return global_msgs
   end

   --
   -- Otherwise get all the current messages,
   -- filter them appropriately, and set the
   -- current selection.
   --
   local msgs = Global:current_messages()

   -- What sort method should we use?
   local method = Config:get("index.sort") or "date"

   if ( method == "date" ) then
      table.sort(msgs, compare_by_date)
   end

   if ( method == "from" ) then
      table.sort(msgs, compare_by_from)
   end

   if ( method == "subject" ) then
      table.sort(msgs, compare_by_subject)
   end

   if ( method == "none" ) then
      -- NOP
   end

   --
   -- Now limit the messages by limit
   --
   local limit = Config:get("index.limit" )

   --
   -- Temporary copies.
   --
   local ret = {}

   if ( limit == nil ) or ( limit == "all" ) then
      for i,o in ipairs(msgs) do
         table.insert(ret, o)
      end

      -- update our cache
      global_msgs = ret
      return ret
   end

   if ( limit == "new" ) then
      for i,o in ipairs(msgs) do
         if ( Message:is_new(o) ) then
            table.insert(ret, o)
         end
      end

      -- update our cache
      global_msgs = ret
      return(ret)
   end

   if ( limit == "today" ) then
      local time = os.time()
      local today = time - ( 60 * 60 * 24 )

      for i,o in ipairs(msgs) do
         -- get current date of the message
         local ctime = Message:to_ctime(o)

         -- if it was within the past 24 hours then add it
         if ( ctime > today ) then
            table.insert(ret, o)
         end
      end

      -- update our cache
      global_msgs = ret
      return(ret)
   end

   if ( limit ) then
      for i,o in ipairs(msgs) do
         local fmt = Message:format(o)
         if ( string.find(fmt, limit) ) then
            table.insert(ret, o)
         end
      end

      -- update our cache
      global_msgs = ret
      return(ret)
   end

   --
   -- NOT REACHED
   --
end



--
-- Simple utility function to test if the given message is
-- new, or unread.
--
function Message:is_new(msg)
   local flags = msg:flags()

   -- If it has the [S]een-flag then it is not new.
   if ( string.find( flags, "S" ) ) then
      return false
   end

   -- If it has the [N]ew-flag then it is new.
   if ( string.find( flags, "N" ) ) then
      return true
   end

   return false
end


--
-- Show the (global) unread-mail count
--
function show_unread()
   local count = 0

   local all = Global:maildirs()
   for i,m in pairs(all) do
      count = count + m:unread_messages()
   end
   Panel:append("Unread messages: " .. count )
end

--
-- Get the current date - used to set the Date: header when
-- we reply, compose, or forward an email.
--
function Message:generate_date()
   return( os.date( "%a, %d %b %Y %H:%M:%S %z" ) )
end


--
-- Return the signature to use for the outgoing mail.
--
function Message:generate_signature()
   --
   -- The sender of our mail.
   --
   local sender = Config:get("global.sender")

   --
   -- Get the home directory, if this fails we'll return
   -- an empty string.
   --
   home = os.getenv( "HOME" )
   if ( not home ) then
      return ""
   end

   -- strip anything except the address, by looking between: <>
   -- i.e. "Steve Kemp" <steve@example.com> becomes steve@example.com
   sender = string.match(sender, "<(.*)>" ) or addr

   -- get the domain, lowercase it.
   domain = string.sub( sender, string.find( sender, "@" )+1 )
   domain = string.lower(domain)

   -- Look for the domain-file ~/.sigs/$domain
   file = home .. "/.sigs/" .. domain
   if ( File:exists( file ) ) then
      return( read_file( file ) )
   end

   --
   -- Fallback to ~/.signature if present
   --
   if ( File:exists( home .. "/.signature" ) ) then
      return( read_file( home .. "/.signature" ) )
   end

   return ""
end


--
-- Compose a new message.
--
function Message:compose()

   -- Get some details
   local to = Screen:get_line("To:")
   if ( not to ) then
      return
   end
   local subject = Screen:get_line("Subject:" )
   if ( not subject ) then
      subject = "No subject"
   end

   -- Get a temporary file, and opening it for writing
   local tmp   = os.tmpname()
   local file  = assert(io.open(tmp, "w"))

   --
   -- Populate the header variables
   --
   local from  = Config:get("global.sender" )
   local msgid = Message:generate_message_id()
   local date  = Message:generate_date()

   -- Write out a header
   header = [[To: ${to}
From: ${from}
Subject: ${subject}
Message-ID: ${msgid}
Date: ${date}

${sig}
]]

   file:write( interp( header, { to      = to,
                                 from    = from,
                                 subject = subject,
                                 msgid   = msgid,
                                 date    = date,
                                 sig     = Message:generate_signature()
                               } ) )

   file:close()

   local run = true

   while( run ) do

      -- Open the editor
      Screen:execute( Config:get( "global.editor" ) .. " " .. tmp )

      -- Once the editor quits ask for an action
      local a = Screen:prompt( "Send message: re(e)dit, (s)end or (c)ancel?", "escESC" )

      if ( a == "s" ) or ( "a" == "S" ) then
         -- Send the mail.
         os.execute( Config:get( "global.mailer" ) .. " < " .. tmp )
         Panel:append("Message sent to " .. to )

         --
         -- Now we need to save a copy of the outgoing message.
         --
         local sent = Config:get( "global.sent-mail" )
         if ( sent ) then
            local msent = Maildir.new( sent )
            local mpath = msent:generate_path( false )
            File:copy(tmp, mpath)
         end

         run = false
      end

      if ( a == 'c' ) or ( a == 'C' ) then
         -- Abort
         Panel:append("Sending aborted!" )
         run = false
      end

      --
      -- a == 'e' is implicit
      --
   end

   --
   -- Remove the temporary-file
   --
   if ( File:exists(tmp ) ) then
      os.remove(tmp)
   end
end


--
-- Reply to the current message
--
function Message:reply()

   --
   -- If we're in message-mode then we can get the current-message
   -- directly.
   --
   -- If instead we're in index-mode then we'll need to select the message
   -- under the cursor to proceed.
   --
   msg = nil

   local mode = Config:get("global.mode")

   if ( mode == "message" ) then
      msg = Global:current_message()
   end
   if ( mode == "index" ) then

      -- Get the list of messages, and the current offset
      -- that'll let us find the message.
      local offset  = Config:get( "index.current" )
      local msgs    = sorted_messages()

      if ( not msgs ) then
         Panel:append( "There are no messages!")
      end
      msg = msgs[offset+1]
   end

   -- Failed to find a mesage?
   if ( not msg ) then
      Panel:append("Failed to find message!")
      return
   end

   -- Get the text of a message.
   local txt = message_view(msg)

   -- Get a temporary file, and opening it for writing
   local tmp  = os.tmpname()
   local file = assert(io.open(tmp, "w"))

   --
   -- Work out who we should be replying to.
   --
   local to = msg:header( "Reply-To" )
   if ( not to ) then
      to = msg:header( "From" )
   end

   --
   -- Build up the subject
   --
   local subject = msg:header( "Subject" )

   --
   -- Remove any (repeated) "Re:" from the start of string.
   --
   while( string.find(subject, "^[rR][eE]:" ) ) do
      subject = string.gsub( subject, "^[rR][eE]:[ \t]+", "" )
   end

   -- Add prefix to the subject.
   subject ="Re: " .. subject

   -- Write out a header
   header = [[To: ${to}
From: ${from}
Subject: ${subject}
Message-ID: ${msgid}
Date: ${date}

]]

   file:write( interp( header, { to      = to,
                                 from    = Config:get("global.sender" ),
                                 subject = subject,
                                 msgid   = Message:generate_message_id(),
                                 date    = Message:generate_date()
                               } ) )


   for i,l in ipairs(txt) do
      file:write( "> " .. l .. "\n")
   end

   -- Append a signature
   file:write( "\n" )
   file:write( Message:generate_signature() )
   file:close()


   local run = true

   while( run ) do

      -- Open the editor
      Screen:execute( Config:get( "global.editor" ) .. " " .. tmp )

      -- Once the editor quits ask for an action
      local a = Screen:prompt( "Send message: re(e)dit, (s)end or (c)ancel?", "escESC" )

      if ( a == "s" ) or ( a == "S" ) then
         -- Send the mail.
         os.execute( Config:get( "global.mailer" ) .. " < " .. tmp )
         Panel:append("Message sent" )

         --
         -- Since we've sent the message we need to add the "(R)eplied"
         -- flag on the source message.
         --
         local cf = msg:flags()
         if ( not string.find( cf, "R" ) ) then
            cf = cf .. "R"
            msg:flags(cf)
         end

         --
         -- Now we need to save a copy of the outgoing message.
         --
         local sent = Config:get( "global.sent-mail" )
         if ( sent ) then
            local msent = Maildir.new( sent )
            local mpath = msent:generate_path( false )
            File:copy(tmp, mpath)
         end

         run = false
      end

      if ( a == "c" ) or ( a == "C" ) then
         -- Abort
         Panel:append("Sending aborted!" )
         run = false
      end

      --
      -- Re-edit is implicit
      --
   end

   --
   -- Remove the temporary-file
   --
   if ( File:exists(tmp ) ) then
      os.remove(tmp)
   end
end


--
-- Delete the current message.
--
function Message:delete()

   --
   -- If we're in message-mode then we can get the current-message
   -- directly.
   --
   -- If instead we're in index-mode then we'll need to select the message
   -- under the cursor to proceed.
   --
   local mode = Config:get("global.mode")

   if ( mode == "message" ) then
      local msg = Global:current_message()
      msg:unlink()
      change_mode("index")

      -- Flush the cached message-list
      global_msgs = {}

      return
   end

   if ( mode == "index" ) then

      -- Get the list of messages, and the current offset
      -- that'll let us find the message.
      local offset  = Config:get( "index.current" )
      local msgs    = sorted_messages()
      if ( not msgs ) then
         Panel:append( "There are no messages!")
      end

      msg = msgs[offset+1]

      -- delete it
      msg:unlink()

      -- now select the previous message.
      offset = offset - 1
      if ( offset < 0 ) then offset = 0 end
      Config:set( "index.current", offset )

      -- Flush the cached message-list
      global_msgs = {}

   end
end


--
-- Forward the current message
--
function Message:forward()

   --
   -- If we're in message-mode then we can get the current-message
   -- directly.
   --
   -- If instead we're in index-mode then we'll need to select the message
   -- under the cursor to proceed.
   --
   msg = nil

   local mode = Config:get("global.mode")

   if ( mode == "message" ) then
      msg = Global:current_message()
   end
   if ( mode == "index" ) then

      -- Get the list of messages, and the current offset
      -- that'll let us find the message.
      local offset  = Config:get( "index.current" )
      local msgs    = sorted_messages()
      if ( not msgs ) then
         Panel:append( "There are no messages!")
      end

      msg = msgs[offset+1]
   end

   -- Failed to find a mesage?
   if ( not msg ) then
      return
   end

   -- Get the text of a message.
   local txt = message_view(msg)

   -- Write it out to to a temporary file
   local tmp  = os.tmpname()
   local file = assert(io.open(tmp, "w"))

   -- Write out a header
   -- Write out a header
   header = [[To: xx@example.com
From: ${from}
Subject: Fwd: ${subject}
Message-ID: ${msgid}
Date: ${date}

Begin forwarded message.

]]

   local from = Config:get("global.sender" )
   local subject = msg:header("Subject")

   file:write( interp( header, { from    = from,
                                 subject = subject,
                                 msgid   = Message:generate_message_id(),
                                 date    = Message:generate_date()
                               } ) )


   for i,l in ipairs(txt) do
      file:write( "> " .. l .. "\n")
   end

   -- Append the signature
   file:write( "\n" )
   file:write( Message:generate_signature() )

   file:close()

   local run = true

   while( run ) do

      -- Open the editor
      Screen:execute( Config:get( "global.editor" ) .. " " .. tmp )

      -- Once the editor quits ask for an action
      local a = Screen:prompt( "Forward message: re(e)dit, (s)end or (c)ancel?", "escESC" )

      if ( a == "s" ) or ( a == "S" ) then

         -- Send the mail.
         os.execute( Config:get( "global.mailer" ) .. " < " .. tmp )
         Panel:append("Message sent" )

         --
         -- Now we need to save a copy of the outgoing message.
         --
         local sent = Config:get( "global.sent-mail" )
         if ( sent ) then
            local msent = Maildir.new( sent )
            local mpath = msent:generate_path( false )
            File:copy(tmp, mpath)
         end

         run = false
      end

      if ( a == "c" ) or ( a == "C" ) then
         -- Abort
         Panel:append("Sending aborted!" )
         run = false
      end


      -- re-edit is implicit
   end

   --
   -- Remove the temporary-file
   --
   if ( File:exists(tmp ) ) then
      os.remove(tmp)
   end
end


--
-- Save a copy of the current message elsehwere.
--
function Message:save()

   --
   -- If we're in message-mode then we can get the current-message
   -- directly.
   --
   -- If instead we're in index-mode then we'll need to select the message
   -- under the cursor to proceed.
   --
   local mode = Config:get("global.mode")
   local msg = nil

   if ( mode == "message" ) then
      msg = Global:current_message()
   end
   if ( mode == "index" ) then

      -- Get the list of messages, and the current offset
      -- that'll let us find the message.
      local offset  = Config:get( "index.current" )
      local msgs    = sorted_messages()
      if ( not msgs ) then
         Panel:append( "There are no messages!")
      end

      msg = msgs[offset+1]
   end

   if ( not msg ) then
      Panel:append( "Failed to find a message" )
      return
   end

   --
   -- Prompt for destination
   --
   local dest = Screen:get_line( "Copy to maildir:",
                                 Config:get( "maildir.prefix" ))
   if ( msg:copy( dest ) ) then
      Panel:append( "Message copied to " .. dest )
   else
      Panel:append( "Message copy failed" )
   end
end


--
-- Utility function to open the Maildir with the given name.
--
-- This matches against the path, and the first match wins.
--
function Maildir.select( desired )

   -- Get the maildirs
   local folders = maildirs()

   -- For each one .. see if it matches
   for index,object in ipairs( folders ) do

      local path = object:path()
      if ( string.find( path, desired ) ) then

         -- Select the maildir, to make it current.
         Global:select_maildir(object)

         -- change the mode, to make it work
         Config:set("global.mode", "index")

         -- Flush the cached message-list
         global_msgs = {}

         -- And update the current selection for when
         -- we return to Maildir-mode.
         Config:set("maildir.current", index -1)
         return
      end
   end
end


--
-- This function is called by pressing `s` in attachment-mode
--
function save_attachment()
   --
   -- Get the currently highlighted attachment-offset
   --
   local mode = Config:get("global.mode")
   local cur  = Config:get(mode .. ".current")

   --
   -- Get the current message, and then the parts.
   --
   local msg   = Global:current_message()
   if ( not msg ) then
      return
   end

   local parts = msg:parts()

   local i = 0
   for k,v in ipairs( parts ) do
      if ( v:is_attachment() ) then
         if ( i == cur ) then

            -- Prompt for local-path
            local output = Screen:get_line( "Save to:", v:filename() )
            -- save it
            local f = io.open(output, "wb")
            f:write( v:content() )
            f:close()

            Panel:append( "Wrote attachment to " .. output )

            return
         end

         i = i + 1
      end
   end
end



--
-- Define our views
--
-----------------------------------------------------------------------------

--
-- This table contains colouring information, it is designed to allow
-- the user to override the colours used in the display easily.
--
colour_table = {}
colour_table['maildir'] = {}
colour_table['index']   = {}
colour_table['message'] = {}


--
-- This function takes a table of lines, and will iterate over
-- every line, updating the strings if we find a match on the
-- regular expressions contained in the colour-table
--
function add_colours( lines, mode )

   --
   -- Get the table of colours for the mode
   -- if it doesn't exist then abort.
   --
   if ( not colour_table ) then return lines end
   if ( not colour_table[mode] ) then return lines  end

   local ret = {}


   --
   -- Iterate over the input-table
   --
   for index,line in ipairs( lines ) do
      --
      -- Iterate over the colours
      --
      for regexp,colour in pairs(colour_table[mode]) do

         if ( string.match( line, regexp ) ) then
            line = colour_line( line, colour )
         end
      end

      table.insert(ret,line)
   end

   return( ret )
end



--
-- This function returns the output which is displayed in attachment-mode
--
-- Attachment mode is entered by pressing `A` when a message is open.
--
function attachment_view()
   local result = {}

   --
   -- Get the parts from within the current message
   --
   local msg = Global:current_message()
   if ( not msg ) then
      return( { "No message selected!" } )
   end

   local parts = msg:parts()

   --
   -- For each one - add it to the display, if it is an attachment.
   --
   for k,v in ipairs( parts ) do
      if ( v:is_attachment() ) then
         local tmp = string.format( "%06d - %32s [%32s]",
                                    v:size(), v:filename(), v:type() )
         table.insert( result, tmp )
      end
   end

   return( result )

end



--
-- This function formats a single message for display in index-mode,
-- it is called by the `index_view()` function defined next.
--
function Message:format(msg)
   local path   = msg:path()
   local time   = msg:mtime()

   -- Do we have this cached?  If so return it
   if ( cache["message:" .. time .. path] ) then
      return(cache["message:" .. time .. path])
   end

   local flags   = msg:flags()
   local subject = msg:header( "Subject" )
   local sender  = msg:header( "From" )

   local output = string.format( "[%4s] - %s - %s", flags, sender, subject )

   --
   -- If the message is unread then show it in the "unread" colour
   --
   if ( string.find( msg:flags(), "N" ) ) then
      output = "$[UNREAD]" .. output
   end

   -- Update the cache.
   cache["message:" .. time .. path] = output

   return( output )
end



--
-- This function displays the screen when in `index`-mode.
--
-- It fetches the list of current messages, and calls `Message:format()`
-- on each one.
--
function index_view()
   local result = {}

   local messages = sorted_messages()

   -- For each one add the output
   for offset,object in ipairs( messages ) do
      local str = Message:format(object)
      table.insert(result,str)
   end

   --
   -- Update the colours
   --
   result = add_colours(result, 'index')
   return result
end



--
-- This function returns the output which is displayed in `lua`-mode.
--
function lua_view()
   --
   -- The output we'll display.
   --
   output = { }

   --
   -- Test of the colour-mode.
   --
   table.insert(output,"$[RED]This $[GREEN]is $[YELLOW]Lumail2 $[CYAN]version $[BLUE]" .. Config:get("global.version" ))
   table.insert(output,"")


   --
   -- Now show the current maildir, message, etc.
   --
   local mode = Config:get("global.mode")
   table.insert(output, "The current mode is" )
   table.insert(output, "$[YELLOW]\t" .. mode )

   table.insert(output, "The currently selected maildir is" )
   local md = Global:current_maildir()
   if ( md ) then
      table.insert(output, "$[RED]\t" .. md:path() )
   else
      table.insert(output, "$[RED]\tUNSET" )
   end

   table.insert(output, "The currently selected message is" )
   local msg = Global:current_message()
   if ( msg ) then
      table.insert(output, "$[RED]\t" .. msg:path() )
   else
      table.insert(output, "$[RED]\tUNSET" )
   end

   --
   -- Now add all the current keys
   --
   table.insert(output, "" )
   table.insert(output, "The following configuration-values have been set:")
   for k,v in ipairs( Config:keys() ) do
      val = Config:get( v )
      table.insert(output, "\t" .. v .. " <" .. type(val) .. ">" )

      if ( ( type(val) == "string" ) or ( type(val) == "number" ) ) then
         table.insert(output, "$[YELLOW]\t\t" .. val )
      end
   end

   --
   -- And return the text.
   --
   return( output )
end


--
-- This method returns the text which is displayed when a maildir is
-- to be show in maildir-mode.
--
-- The actual output of maildir-mode is generated by `maildir_view`
-- which is defined below.
--
function Maildir:format(obj)
   local path   = obj:path()
   local time   = obj:mtime()
   local trunc  = Config:get("truncate.maildir") or 0

   -- Do we have this cached?  If so return it
   if ( cache["maildir:" .. trunc .. time .. path] ) then
      return(cache["maildir:" .. trunc .. time .. path])
   end

   local total  = obj:total_messages()
   local unread = obj:unread_messages()

   --
   -- Path might be truncated, via "p".
   --
   if ( trunc ~= 0 ) then
      path = File:basename(path)
   end

   local output = string.format( "[%05d / %05d] - %s", unread, total, path )

   --
   -- If there are unread messages then show it in the unread-colour.
   --
   if ( unread > 0 ) then
      output = "$[UNREAD]" .. output
   end

   -- update the cache
   cache["maildir:" .. trunc .. time .. path] = output

   return output
end


--
-- This method returns the text which is displayed in maildir-view
--
-- It retrieves the list of Maildirs, and calls `Maildir:format` on
-- each one.
--
function maildir_view()
   local result = {}

   -- Get the maildirs
   local folders = maildirs()

   -- For each one add the output
   for index,object in ipairs( folders ) do
      local str = Maildir:format(object)
      table.insert(result,str)
   end

   --
   -- Update the colours
   --
   result = add_colours(result, 'maildir')
   return result
end



--
-- This method returns the text which is displayed in message-mode.
--
-- First of all the current-message is retrieved, then that is
-- formatted into an array of lines which are displayed to the user.
--
-- The scrolling is handled on the C++ side.
--
function message_view( msg )

   --
   -- If we're called in `message`-mode then we'll not have an
   -- argument, so we need to find the message.
   --
   if ( not msg ) then
      msg = Global:current_message()
   end

   if ( not msg ) then
      return({"No message selected!"})
   end

   --
   -- Change the message to being read, if it is new.
   --
   if ( Message:is_new( msg ) ) then
      msg:mark_read()
   end

   local output = ""

   --
   -- The headers we want to see by default.
   --
   headers = { "To", "From", "Cc", "Subject", "Date" }

   --
   -- For each header, get it, and the value.
   --
   for i,header in ipairs( headers ) do
      value = msg:header(header) or "[unset]"
      if ( value  ) then
         output = output ..  header .. ": " .. value .. "\n"
      end
   end

   --
   -- Add some text.
   --
   output = output .. "\n"

   --
   -- Now look for the first text/* content-type
   --
   parts = msg:parts()

   local found = false
   for i,part in ipairs( parts ) do
      if ( found == false ) then

         --
         -- Get the content-type of this part.
         --
         ct = part:type():lower()

         if ( string.find( ct, "text/" ) ) then
            output = output .. part:content()
            found = true
         end
      end
   end

   --
   -- Did we show the body?
   --
   if ( found == false ) then
      output = output .. "Failed to find a 'text/xxx' part from the message."
   end

   --
   -- The command output is now split into rows.
   --
   local result = string_to_table( output )

   --
   -- Update the colours
   --
   result = add_colours(result, 'message')
   return(result)
end




--
-- Define some functions which are bound to keys, to move around, etc.
--
-----------------------------------------------------------------------------


--
-- Read input, and evaluate it as lua.
--
function read_eval()
   local txt = Screen:get_line(":")
   loadstring( txt )()
end


--
-- Read input, and evaluate it as a shell-command.
--
function read_execute()
   local cmd = Screen:get_line("!")
   os.execute(cmd)
end



-- Allow navigation - Selection of a maildir, or message.
--
function select()
   local mode = Config:get("global.mode")
   local cur  = Config:get(mode .. ".current")

   if ( mode == "maildir" ) then
      local folders = maildirs()
      local folder  = folders[cur+1]
      Global:select_maildir( folder )

      -- Log the change of maildir.
      Panel:append( "Selected maildir " .. folder:path() )

      --
      -- The user might want to change email addresses
      -- now
      for pattern,email in pairs(folder_from) do
         if ( string.find( folder:path(), pattern ) )then
            Config:set("global.sender", email )
            Panel:append("Changed outgoing email-address to " .. email )
         end
      end


      --
      -- Change to the index-mode, so we can see the messages in
      -- the folder.
      --
      Config:set("global.mode", "index" )

      --
      -- Flush the cached message-list
      --
      global_msgs = {}

      return
   end

   if ( mode == "index" ) then
      --
      -- Get the messages
      --
      local msgs = sorted_messages()

      --
      -- Get the current offset.
      --
      local msg  = msgs[cur+1]

      --
      -- Now select
      --
      Global:select_message( msg )
      Config:set("global.mode", "message" )
      return
   end
end


--
-- Jump to the first entry in the current-mode.
--
function first()
   local mode = Config:get("global.mode")
   Config:set( mode .. ".current", 0)
end


--
-- Jump to the last entry in the current-mode.
--
function last()
   local mode = Config:get("global.mode")
   local max  = Config:get(mode .. ".max" )

   Config:set( mode .. ".current", max-1 )
end


--
-- Left/Right scrolling
--
function left()
   local x = Config:get("global.horizontal")
   if ( not x ) then
      x = 0
   end

   if ( x > 0 ) then
      x = x - 1
   end

   Config:set( "global.horizontal", x )
end


--
-- Left/Right scrolling.
--
function right()
   local x = Config:get("global.horizontal")

   if ( not x ) then x = 0 end
   x = x + 1

   Config:set( "global.horizontal", x )
end


--
-- Allow searching for the next line containing a match.
--
-- This function might need some care to understand:
--
--  * The mail-client is a modal editor.
--  * You're always in one mode, stored in "Config:get("global.mode")".
--  * Each mode displays the output of the lua-function $mode_view()
--  * This function returns a table of lines.
--  * There are also "current-offset" and "max-size" variables for these tables.
--  * We can use this to dynamically invoke the right mode, and iterate.
--
--
function find( offset )

   -- Get the thing we're searching for
   local pattern = Screen:get_line( "/:" )

   -- Get the global mode.
   local mode = Config:get("global.mode")

   -- Use that to get the lines we're currently displaying
   loadstring( "out = " .. mode .. "_view()" )()

   --
   -- We know the current offset is stored in
   -- the variable $mode.current
   --
   -- We know the maximum offset is stored in the
   -- variable $mode.max
   --
   local cur = Config:get(mode .. ".current") or 0
   local max = Config:get(mode .. ".max")

   --
   -- We'll keep track of how many times we've moved forward
   -- to avoid looping indefinitely.
   --
   local count = -1

   cur = cur + 1

   --
   -- Start searching from the current-position
   --
   while( count < max ) do

      cur = cur + offset
      if ( cur > max ) then cur = 1 end
      if ( cur < 1 )   then cur = max end

      -- Get the current entry
      local line = out[cur]

      -- Does it match?
      if ( string.match( line, pattern ) ) then
         Config:set(mode .. ".current", (cur - 1))
         return
      end

      count = count + 1
   end
end



--
-- Scroll the current mode down - by manipulating the "current" offset
-- for the current mode.
--
function next( offset )

   local mode = Config:get("global.mode")
   local cur  = Config:get(mode .. ".current")
   local max  = Config:get(mode .. ".max" )

   if ( not cur ) then
      cur = 0
   end

   if ( cur + offset < (max-1) ) then
      cur = cur + offset
   else
      cur = max-1
   end

   if ( cur >= max ) then cur = max end

   Config:set( mode .. ".current", cur )
end


--
-- Scroll the current mode up - by manipulating the "current" offset
-- for the current mode.
--
function prev( offset )
   local mode = Config:get("global.mode")
   local cur  = Config:get(mode .. ".current")

   if ( not cur ) then
      cur = 0
   end

   if ( cur - offset > 0 ) then
      cur = cur - offset
   else
      cur = 0
   end
   Config:set( mode .. ".current", cur )
end


--
-- Toggle the panel height between "small" and "large".
--
function panel_size_toggle()
   local h = Panel:height()

   if ( h == 6  ) then
      h = 16
   else
      h = 6
   end

   Panel:height(h)
end


--
-- This function jumps to the previous message, if possible.
--
-- It is only called when viewing a single message.
--
function prev_message()

   -- Get the messages, and sort.
   local msgs = sorted_messages()

   -- Get the current offset
   local cur = Config:get("index.current")

   if ( cur > 0 ) then
      cur = cur - 1
      Global:select_message( msgs[cur] )
      Config:set("index.current",cur)
   end
end

--
-- This function jumps to the next message, if possible.
--
-- It is only called when viewing a single message.
--
function next_message()

   -- Get the messages, and sort.
   local msgs = sorted_messages()

   -- Get the current offset + max
   local cur = Config:get("index.current")
   local max = Config:get("index.max")

   if ( cur < max ) then
      cur = cur + 1
      Global:select_message( msgs[cur] )
      Config:set("index.current",cur)
   end
end


--
-- Define some call-backs which are implemented at various times.
--
-----------------------------------------------------------------------------


--
-- This function is called to generate TAB-completion results.
--
-- Given a token the user has entered it should return a table
-- containing all possible matches.  The user-interface will
-- complete precisely if there is a single entry in the table, if not
-- it will prompt the user to choose from the available selection.
--
function on_complete( token )

   --
   -- Some fixed things that we should be able to complete upon.
   --
   tmp = {}

   --
   -- The values we'll return to the caller.
   --
   ret = { }

   --
   -- Add in our local objects.
   --
   local objs = {}
   objs["Config:"]    = Config
   objs["Directory:"] = Directory
   objs["File:"]      = File
   objs["Global:"]    = Global
   objs["Maildir:"]   = Maildir
   objs["Message:"]   = Message
   objs["Net:"]       = Net
   objs["Panel:" ]    = Panel
   objs["Screen:" ]   = Screen

   for name, object in pairs(objs) do
      for key,value in pairs(object) do
         if ( not string.match( key, "^_" ) ) then
            tmp[ name .. key ] = name .. key
         end
      end
   end

   --
   -- Add in all user-defined functions.
   --
   for k,v in pairs(_G) do
      tmp[k] = k
   end

   --
   -- If the token starts with "~" then replace that with
   -- the users' home-directory
   --
   if ( string.sub( token, 0, 1 ) == "~" ) then
      token = string.sub( token, 2 )
      token = os.getenv("HOME") .. token
   end


   --
   -- Is the user attempting to complete on a file-path?
   --
   if ( string.match( token, "^/" ) ) then

      --
      -- Get the directory this is from.
      --
      -- Default to / if we found no match.
      --
      dir = string.match(token, "^(.*)/" )
      if ( dir == "" ) then dir = "/" end

      --
      -- If the directory exists then add all the entries to the completion-set.
      --
      if ( File:exists( dir ) ) then
         entries = Directory:entries( dir )
         for i,v in ipairs(entries) do
            tmp[v] = v
         end
      end
   end

   --
   -- Do we have a match?
   --
   for k,v in pairs(tmp) do
      if ( string.match( v, "^" .. token ) ) then
         table.insert(ret, v)
      end
   end

   --
   -- Return the sorted value(s).
   --
   table.sort(ret)
   return(ret)

end


--
-- This function is called when the main-loop timesout, which is roughly
-- once per second.
--
do
   -- last sync time.
   local ls = os.time()

   function on_idle()

      -- get current time
      ct = os.time()

      -- If it is two minutes then save our cache to disk
      if ( ( ct - ls ) >= ( 60 * 2 ) ) then
         ls = ct
         cache_save()
      end

      --
      -- This shows the way that we can append messages constantly
      -- to the panel.
      --
      -- Panel:append("The date is " .. Message:generate_date())
      --
   end
end


--
-- If this function is defined it will be invoked any time
-- a configuration-key has its value changed.
--
-- The single argument will be the name of the key which has
-- been updated - the value can be retrieved via Config:get, but
-- remember that the value might be a string or a table.
--
function Config.key_changed( name )

   --
   -- If the index.limit value has changed then we can
   -- update our state.
   --
   if ( name == "index.limit" ) then
      global_msgs = {}
      Panel:append("Flushed message cache.")
   end

end




--
--  Define our key-bindings.
--
-----------------------------------------------------------------------------

--
-- Setup our KeyMaps
--
keymap = {}
keymap['attachment'] = {}
keymap['global']     = {}
keymap['maildir']    = {}
keymap['index']      = {}
keymap['message']    = {}

--
-- Global keybindings, which work in all modes.
--
keymap['global'][':']   = "read_eval()"
keymap['global']['!']   = "read_execute()"
keymap['global']['TAB'] = "Panel:toggle()"
keymap['global']['M']   = "change_mode( 'maildir' )"
keymap['global']['I']   = "change_mode( 'index' )"
keymap['global']['L']   = "change_mode( 'lua' )"


--
-- Next/Previous navigation for different modes
--
keymap['global']['KEY_UP']   = "prev(1)"
keymap['global']['k']        = "prev(1)"
keymap['global']['j']        = "next(1)"
keymap['global']['KEY_DOWN'] = "next(1)"

keymap['global']['K']         = "prev(10)"
keymap['global']['KEY_PPAGE'] = "prev(10)"
keymap['global']['J']         = "next(10)"
keymap['global']['KEY_NPAGE'] = "next(10)"

keymap['global']['<']        = "first()"
keymap['global']['KEY_HOME'] = "first()"
keymap['global']['*']        = "last()"
keymap['global']['>']        = "last()"
keymap['global']['KEY_END']  = "last()"

keymap['global']['ENTER'] = "select()"
keymap['global']['SPACE'] = "select()"

--
-- Message-mode allows J/K to move to next/prev message
--
keymap['message']['J'] = 'next_message()'
keymap['message']['K'] = 'prev_message()'

--
-- Left/Right scrolling.  (Global)
--
keymap['global']['h']         = "left()"
keymap['global']['KEY_LEFT']  = "left()"
keymap['global']['l']         = "right()"
keymap['global']['KEY_RIGHT'] = "right()"

--
-- Forward/backward-searching (Global)
--
keymap['global']['/'] = 'find(1)'
keymap['global']['?'] = 'find(-1)'

--
-- Change the display-limits
--
keymap['maildir']['a'] = 'Config:set( "maildir.limit", "all" )'
keymap['maildir']['n'] = 'Config:set( "maildir.limit", "new" )'
keymap['maildir']['t'] = 'Config:set( "maildir.limit", "today" )'

--
-- Limit the display of messages appropriately
--
keymap['index']['a']   = 'Config:set( "index.limit", "all" )'
keymap['index']['n']   = 'Config:set( "index.limit", "new" )'
keymap['index']['t']   = 'Config:set( "index.limit", "today" )'

--
-- Force a cache flush - via a sleazy hack.
--
keymap['global']['^L'] = 'Config.key_changed( "index.limit" )'

--
-- Exit out of modes
--
keymap['maildir']['q']    = "os.exit()"
keymap['index']['q']      = "change_mode('maildir')"
keymap['message']['q']    = "change_mode('index')"
keymap['attachment']['q'] = "change_mode('message')"

--
-- Enter attachment-mode
--
keymap['message']['A']    = "change_mode('attachment')"

--
-- Save the current attachment
--
keymap['attachment']['s'] = "save_attachment()"


--
-- Actions relating to messages.
--
keymap['global']['c']  = 'Message:compose()'
keymap['message']['r'] = 'Message:reply()'
keymap['index']['r']   = 'Message:reply()'
keymap['message']['f'] = 'Message:forward()'
keymap['index']['f']   = 'Message:forward()'
keymap['message']['d'] = 'Message:delete()'
keymap['index']['d']   = 'Message:delete()'
keymap['message']['s'] = 'Message:save()'
keymap['index']['s']   = 'Message:save()'

--
-- Toggle display of full maildir paths
--
keymap['maildir']['p'] = 'toggle_variable( "truncate.maildir" )'

--
-- Toggle the size of the panel
--
keymap['global']['P'] = 'panel_size_toggle()'



--
--  Configure the mail-client.
--
-----------------------------------------------------------------------------

--
-- The default Maildir location is ~/Maildir
--
local def_maildir = os.getenv( "HOME" ) .. "/Maildir"

--
-- Set the maildir-prefix, if it exists.
--
if ( Directory:is_maildir(def_maildir) ) then
   Config:set( "maildir.prefix", def_maildir )
else
   Panel:append( "WARNING: No maildir-prefix defined!" )
end

--
-- Configure the path to save outgoing messages to.
--
local def_save = os.getenv( "HOME" ) .. "/Maildir/sent-mail"
if (Directory:is_maildir(def_save) ) then
   Config:set( "global.sent-mail", def_save)
else
   Panel:append( "WARNING: No sent-mail folder defined!" )
end


--
-- Setup our MTA
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
Config:set( "global.sender", "Steve Kemp <steve@steve.org.uk>" )

--
-- Unread messages/maildirs are drawn in red.
--
Config:set( "colour.unread", "red" )

--
-- Save persistant history of our input in the named file.
--
Config:set( "global.history", os.getenv( "HOME" ) .. "/.lumail2.history" )

--
-- Configure a cache-file, and populate it
--
Config:set( "message.cache", os.getenv( "HOME" ) .. "/.lumail2.cache" )
cache_load()



--
-- Some people like ot change email addresses when they change
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
   ['Automated'] = 'yellow',
   ['lists']     = 'green',
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
   ['^> ']  = 'yellow',
   ['^> >'] = 'green'
}




--
--   Handle any command-line argumenst
--
-----------------------------------------------------------------------------

for index,arg in ipairs(ARGS) do

   --
   -- Look for --folder=foo
   --
   local folder = string.match(arg, "--folder=(.*)" )
   if ( folder ) then
      Maildir.select( folder )
   end

   --
   -- Look for --eval=bar()
   --
   local txt = string.match(arg, "--eval=(.*)" )
   if ( txt ) then
      f = loadstring( txt )
      f()
   end

end
