
--
-- Define some utility functions
--


--
-- This method is CRUCIAL to our operation.
--
-- This method returns the text which is displayed when a specific
-- message-object is displayed.
--
-- If you want to perform HTML->Text conversion, apply filters, or do
-- other such things you must override this method.
--
function Message.to_string(self)
   local output = ""

   --
   -- The headers we want to see by default.
   --
   headers = { "To", "From", "Cc", "Subject", "Date" }

   --
   -- For each header, get it, and the value.
   --
   for i,header in ipairs( headers ) do
      value = self:header(header) or "[unset]"
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
   parts = self:parts()

   local found = false
   for i,part in ipairs( parts ) do
      if ( found == false ) then

         --
         -- Get the content-type of this part.
         --
         ct = part:type()

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

   return( output )
end



--
-- This method is CRUCIAL to our operation.
--
-- This method returns the text which is displayed when a maildir is
-- to be show in maildir-mode.
--
function Maildir.to_string(self)
   local total = self:total_messages()
   local unread = self:unread_messages()
   local path   = self:path()

   local output = string.format( "[%05d / %05d] - %s", unread, total, path );

   if ( unread > 0 ) then
      output = "$[RED]" .. output
   end
   return output
end


--
-- This is utility-function for showing some output in the status-panel.
--
function show_command_output( title, cmd )
   Panel:title("Command Output");

   local handle = io.popen(cmd)
   local result = handle:read("*a")
   handle:close()

   Panel:text( { title, "\t" .. result } )
   if ( Panel:visible() == false ) then
      Panel:show()
   end
end

--
-- Show the date in the panel
--
function date()
   show_command_output( "The date is", "date" );
end

--
-- Show our hostname in the panel.
--
function hostname()
   show_command_output( "The host is", "hostname" );
end

--
-- Read input, and evaluate it.
--
function read_eval()
   local txt = Screen:get_line();
   loadstring( txt )()
end

--
-- This animates the title of the display-panel, which is an interesting
-- effect!
--
function on_idle()
   title = Panel:title()
   local tmp = string.sub(title, 2 )
   tmp = tmp .. string.sub(title,1,1)
   Panel:title(tmp)
end


--
-- If this function is defined it will be invoked any time
-- a configuration-key has its value changed.
--
-- The single argument will be the name of the key which has
-- been updated - the value can be retrieved via Config:get
--
--[[
function Config.key_changed( name )
   print( "Key changed: " .. name)
end
--]]


--
-- Change the mode - and update the panel-title, if we have one.
--
function change_mode( new_mode )
   Config:set( "global.mode", new_mode )
   Panel:title( "Mode is now " .. new_mode .. " " )
end


--
-- Setup our KeyMaps
--
keymap = {}
keymap['global']  = {}
keymap['demo']    = {}
keymap['maildir'] = {}
keymap['index']   = {}
keymap['message'] = {}

--
-- Global Mode
--
keymap['global'][':'] = "read_eval()"
keymap['global']['\t'] = "Panel:toggle()"


keymap['global']['M'] = "change_mode( 'maildir' );"
keymap['global']['I'] = "change_mode( 'index' );"
keymap['global']['D'] = "change_mode( 'demo' );"


--
-- Demo Mode-specific bindings
--
keymap['demo']['d' ] = "date()"
keymap['demo']['h' ] = "hostname()"



--
-- Setup the prefix to our maildir hierarchy
--
Config:set( "maildir.prefix", os.getenv( "HOME" ) .. "/Maildir" )
