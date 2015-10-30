
--
-- Define some utility functions
--


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
function string_to_table(str)
  local t = {}
  local function helper(line) table.insert(t, line) return "" end
  helper((str:gsub("(.-)\r?\n", helper)))
  return t
end


--
-- Remove a colour-string from the given string
--
-- A colour-string is something like "$[BLUE]" at the
-- start of a line.
--
function strip_colour( input )
   while( string.find(input, "^$[^]]+]" ) ) do
      input = string.gsub( input, "^$[^]]+]", "" )
   end
   return input
end






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

   --
   -- The command output is now split into rows.
   --
   return( string_to_table( output ) )
end


--
-- This method is CRUCIAL to our operation.
--
-- This method returns the index-entry for a message in `index`-mode.
--
function Message.to_index(self)
   local flags = self:flags()
   local subject = self:header( "Subject" )
   local sender  = self:header( "From" )

   local output = string.format( "[%4s] - %s - %s", flags, sender, subject )

   --
   -- If the message is unread then show it in RED
   --
   if ( string.find( self:flags(), "N" ) ) then
      output = "$[RED]" .. output
   end

   --
   -- If the message contains the word "STEVE" - show it in blue
   --
   if ( string.find( output, "steve" )  ) then
      output = strip_colour( output )
      output = "$[BLUE]" .. output
   end


   return( output );
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

   if ( string.find( output, "Automated." ) ) then
      output = strip_colour( output )
      output = "$[YELLOW]" .. output
   end
   if ( string.find( output, "lists" ) ) then
      output = strip_colour( output )
      output = "$[GREEN]" .. output
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
-- Get output for Lua-mode
--
function lua_mode()
   --
   -- We show this first
   --
   a = {
      "$[RED]This is a red line",
      "$[BLUE]This is a blue line",
      "$[GREEN]This is a green line",
   }

   --
   -- Now we get some text from running a command.
   --
   local handle = io.popen("cat /etc/passwd")
   local result = handle:read("*a")
   handle:close()

   --
   -- The command output is now split into rows.
   --
   result = string_to_table( result )

   --
   -- Add the command output to the original table.
   --
   for k,v in ipairs( result ) do
      if ( string.find( v, "root" ) or
	   string.find( v, "nobody" ) ) then
	 v = "$[CYAN]" .. v
	 end
      table.insert( a, v )
   end

   --
   -- And return the text.
   --
   return( a )
end



-- Allow navigation - Selection of a maildir, or message.
--
function select()
   local mode = Config:get("global.mode")
   local cur  = Config:get(mode .. ".current")

   if ( mode == "maildir" ) then
      Screen:select_maildir( cur )
      Config:set("global.mode", "index" )
      return
   end
   if ( mode == "index" ) then
      Screen:select_message( cur )
      Config:set("global.mode", "message" )
      return
   end
end



--
-- Jump to first/last entries.
--
function first()
   local mode = Config:get("global.mode")
   Config:set( mode .. ".current", "0" )
end

function last()

   local mode = Config:get("global.mode")
   local max  = Config:get(mode .. ".max" )

   max = tonumber( max )
   Config:set( mode .. ".current", max-1 )
end


--
-- Left/Right scrolling
--
function left()
   local x = Config:get("global.horizontal")
   if ( not x ) then
      x = 0
   else
      x = tonumber( x )
   end

   if ( x > 0 ) then
      x = x - 1
   end

   Config:set( "global.horizontal", x )
end

function right()
   local x = Config:get("global.horizontal")
   if ( not x ) then
      x = 0
   else
      x = tonumber( x )
   end

   if ( x < Screen:width() ) then
      x = x + 1
   end
   Config:set( "global.horizontal", x )
end


--
-- Allow navigation - Scroll down a maildir, or index.
--
function next( offset )

   local mode = Config:get("global.mode")
   local cur  = Config:get(mode .. ".current")
   local max  = Config:get(mode .. ".max" )

   if ( not cur ) then
      cur = 0
   else
      cur = tonumber( cur )
   end

   max = tonumber( max )

   if ( cur + offset < (max-1) ) then
      cur = cur + offset
   else
      cur = max-1
   end

   if ( cur >= max ) then cur = max end

   Config:set( mode .. ".current", cur )
end


--
-- Allow navigation - Scroll up a maildir, or index.
--
function prev( offset )
   local mode = Config:get("global.mode")
   local cur  = Config:get(mode .. ".current")

   if ( not cur ) then
      cur = 0
   else
      cur = tonumber( cur )
   end

   max = tonumber( max )

   if ( cur - offset > 0 ) then
      cur = cur - offset
   else
      cur = 0
   end
   Config:set( mode .. ".current", cur )
end


--
-- This function is called to generate tab-completions
--
function on_complete( token )

   --
   -- Some fixed things that we should be able to complete upon.
   --
   tmp = {
      "Panel:title", "Panel:text", "Screen:exit"
   }

   --
   -- The values we'll return to the caller.
   ret = { }

   --
   -- Add in all user-defined functions.
   --
   for k,v in pairs(_G) do
      tmp[k] = k
   end

   --
   -- Do we have a match?
   --
   for k,v in pairs(tmp) do
      if ( string.match( v, "^" .. token ) ) then
         ret[k] = v
      end
   end

   --
   -- Return the value(s).
   --
   return(ret)
end


--
-- This animates the title of the display-panel, which is an interesting
-- effect.  Or an annoyance, depending on which you prefer.
--
function on_idle()
--[[
   --
   -- Get the title, shift it once byte, and set it
   --
   title = Panel:title()
   local tmp = string.sub(title, 2 )
   tmp = tmp .. string.sub(title,1,1)
   Panel:title(tmp)

   --
   --  Show some information in the panel.
   --
   local mode = Config:get("global.mode")
   local cur  = Config:get(mode .. ".current")
   if not cur  then
      cur = "UNSET"
   end
   local max  = Config:get(mode .. ".max" )
   if not max  then
      max = "UNSET"
   end

   local text  = "Current mode "..mode.." Current offset:".. cur .." Max offset:".. max
   local text2 =  "$[YELLOW]This is lumail2, by Steve Kemp"

   if ( mode == "index" ) then
      local md = Screen:maildir()
      text2 = "Current maildir : " .. md:path()
   end
   if ( mode == "message" ) then
      local md = Screen:message()
      text2 = "Current message : " .. md:path()
   end
   Panel:text( { text, text2 } )
--]]
   Panel:title( "$[RED]Red title here. This must be serious!" )
   Panel:text( { "$[YELLOW]Yellow line here.", "$[GREEN]Green line here!" } )
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
   Panel:title( "Mode is now " .. new_mode )
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
keymap['global']['TAB'] = "Panel:toggle()"


keymap['global']['M'] = "change_mode( 'maildir' );"
keymap['global']['I'] = "change_mode( 'index' );"
keymap['global']['D'] = "change_mode( 'demo' );"
keymap['global']['L'] = "change_mode( 'lua' );"


--
-- Next/Previous navigation for different modes
--
keymap['global']['KEY_UP']   = "prev(1)"
keymap['global']['k']        = "prev(1)"
keymap['global']['j']        = "next(1)"
keymap['global']['KEY_DOWN'] = "next(1)"

keymap['global']['K'] = "prev(10)"
keymap['global']['KEY_PPAGE'] = "prev(10)"
keymap['global']['J'] = "next(10)"
keymap['global']['KEY_NPAGE'] = "next(10)"

keymap['global']['*'] = "last()"
keymap['global']['<'] = "first()"
keymap['global']['>'] = "last()"

keymap['global']['ENTER'] = "select()"

--
-- Left/Right scrolling.  (Global)
--
keymap['global']['h']         = "left()";
keymap['global']['KEY_LEFT']  = "left()";
keymap['global']['l']         = "right()";
keymap['global']['KEY_RIGHT'] = "right()";

--
-- Change the display-limits
--
keymap['maildir']['a'] = 'Config:set( "maildir.limit", "all" )'
keymap['maildir']['n'] = 'Config:set( "maildir.limit", "new" )'

--
-- Change the display-limits
--
keymap['index']['a'] = 'Config:set( "index.limit", "all" )'
keymap['index']['n'] = 'Config:set( "index.limit", "new" )'

--
-- Exit out of modes
--
keymap['index']['q']   = "change_mode('maildir')"
keymap['message']['q'] = "change_mode('index')"

--
-- Demo Mode-specific bindings
--
keymap['demo']['d' ] = "date()"
keymap['demo']['h' ] = "hostname()"



--
-- Setup the prefix to our maildir hierarchy.
--
Config:set( "maildir.prefix", os.getenv( "HOME" ) .. "/Maildir" )

--
-- Setup our default editor.  Not used.
--
Config:set( "global.editor", "vim  +/^$ ++1 '+set tw=72'" )

--
-- Save persistant history of our input in the named file.
--
Config:set( "global.history", os.getenv( "HOME" ) .. "/.lumail2.history" )
