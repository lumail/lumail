--
-- Dump the messages in named-folders.
--
-- Usage:
--
--     lumail2 --no-curses --load-file ./dump_maildir.lua
--

function show_folder( path )

   print( "Looking for the folder:"  .. path )

   --
   -- Get all known Maildirs
   --
   local mdirs = Global:maildirs()

   --
   -- For each one, does the patch match?
   --
   for i,m in ipairs(mdirs) do

      if ( string.match(m:path(), path ) ) then
         print( "Found maildir " .. m:path() )
         print( "\tThere are " .. m:total_messages() .. " total messages." )
         print( "\tThere are " .. m:unread_messages() .. " unread messages." )

         -- Now get the messages
         local msgs = m:messages()

         for j,o in ipairs(msgs) do
            print( "\t\t" .. o:header("Subject" ) )
         end

         return
      end
   end

   --
   -- Failed to find Maildir
   --
   print("Failed to find Maildir matching " .. path )
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
