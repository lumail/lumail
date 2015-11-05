--
-- Dump the messages in named-folders.
--
-- Usage:
--
--     ./lumail2 --no-curses --load-file ./dump_maildir.lua
--

function show_folder( path )

   print( "Looking for the folder:"  .. path )

   --
   -- Get the max folders
   --
   local cur = 0
   local max = tonumber(Config:get( "maildir.max" ))

  
   while( cur < tonumber(max) ) do
      -- Select the maildir - and get it
      Screen:select_maildir( cur) 
      local m = Screen:maildir()

      -- Does this path match waht we wanted?
      if ( string.find( m:path(),  path )  ) then
	 found = cur
      end

      -- Loop again.
      cur = cur + 1
   end

   -- Failed to find it.
   if ( found == -1 ) then
      print( "Failed to find folder with name:" .. path )
      return
   end

   --
   -- Select the maildir
   --
   Screen:select_maildir(found)

   --
   -- Count the messages
   --
   Config:set( "global.mode", "index" )
   max = Config:get( "index.max" )
   cur = 0
   print(" There are " .. max .. " messages" )

   while( cur < tonumber(max) ) do
      -- Select the maildir - and get it
      Screen:select_message( cur) 
      local m = Screen:message()

      print( "\tMessage " .. cur .. " has Subject:" ..  m:header( "Subject" ) )
      -- Loop again.
      cur = cur + 1
   end
   
   
end


--
-- Load our maildir hierarchy.
--
Config:set( "maildir.prefix", os.getenv( "HOME" ) .. "/Maildir" )

--
-- Show some folder(s).
--
show_folder( "Hip.Chat" )
show_folder( "lists.dc" )
