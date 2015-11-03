
--
-- 1. Define some utility functions
--
-----------------------------------------------------------------------------
-----------------------------------------------------------------------------


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
-- Remove a colour-prefix from the given string
--
-- Our drawing code allows lines to be different coloured based upon
-- a prefix.  So for example the text "$[RED]I like Helsinki" would be
-- drawn as "I like Helsinki", in the colour red.
--
-- This function removes any existing prefix, to avoid blind additions
-- that might end up like "$[RED]$[YELLOW]I like cakes".
--
function strip_colour( input )
   while( string.find(input, "^$[^]]+]" ) ) do
      input = string.gsub( input, "^$[^]]+]", "" )
   end
   return input
end


--
-- Change the mode - and update the panel-title, if we have one.
--
function change_mode( new_mode )
   Config:set( "global.mode", new_mode )
   Panel:title( "Mode is now " .. new_mode )
end


--
-- Simple utility function to test if the given message is
-- new, or unread.
--
function Message:is_new(msg)
   local flags = msg:flags()

   -- If it has the [S]een-flag then it is not new.
   if ( string.find( flags, "S" ) ) then
      return false;
   end

   -- If it has the [N]ew-flag then it is new.
   if ( string.find( flags, "N" ) ) then
      return true
   end

   return false
end


--
-- Utility function to open the Maildir with the given name.
--
-- This matches against the path, and the first match wins.
--
function Maildir.select( desired )

   -- Get the maildirs
   local folders = current_maildirs()

   -- For each one .. see if it matches
   for index,object in ipairs( folders ) do
      local path = object:path()
      if ( string.find( path, desired ) ) then

         -- update the selection for when we return
         Config:set("maildir.current", index -1)

         -- select the maildir
         Screen:select_maildir( index -1 )

         -- change the mode
         Config:set("global.mode", "index")
         return
      end
   end
end


--
-- 2. Define our views
--
-----------------------------------------------------------------------------


--
-- This function formats a single message for display in index-mode,
-- it is called by the `index_view()` function defined next.
--
function Message:format(msg)
   local flags   = msg:flags()
   local subject = msg:header( "Subject" )
   local sender  = msg:header( "From" )

   local output = string.format( "[%4s] - %s - %s", flags, sender, subject )

   --
   -- If the message is unread then show it in RED
   --
   if ( string.find( msg:flags(), "N" ) ) then
      output = "$[RED]" .. output
   end

   --
   -- If the message contains the word "STEVE" - show it in blue
   --
   if ( string.find( output, "steve" )  ) then
      output = strip_colour( output )
      output = "$[BLUE]" .. output
   end
   return( output )
end


--
-- This function returns the output which is displayed in attachment-mode
--
function attachment_view()
   local result = {}

   --
   -- Get the parts from within the current message
   --
   local msg   = current_message()
   local parts = msg:parts()

   --
   -- For each one - add it in
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
-- This function displays the screen when in `index`-mode.
--
-- It fetches the list of current messages, and calls `Message:format()`
-- on each one.
--
function index_view()
   local result = {}

   -- Get the messages in the maildir
   local messages = messages()

   -- For each one add the output
   for offset,object in ipairs( messages ) do
      local str = Message:format(object)
      table.insert(result,str)
   end

   return result
end



--
-- This function returns the output which is displayed in `lua`-mode.
--
function lua_view()
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



--
-- This method returns the text which is displayed when a maildir is
-- to be show in maildir-mode.
--
-- The actual output of maildir-mode is generated by `maildir_view`
-- which is defined below.
--
function Maildir:format(obj)
   local total  = obj:total_messages()
   local unread = obj:unread_messages()
   local path   = obj:path()

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
-- This method returns the text which is displayed in maildir-view
--
-- It retrieves the list of Maildirs, and calls `Maildir:format` on
-- each one.
--
function maildir_view()
   local result = {}

   -- Get the maildirs
   local folders = current_maildirs()

   -- For each one add the output
   for index,object in ipairs( folders ) do
      local str = Maildir:format(object)
      table.insert(result,str)
   end

   return result
end



--
-- This method returns the text which is displayed in mode-mode.
--
-- First of all the current-message is retrieved, then that is
-- formatted into an array of lines which are displayed to the user.
--
-- The scrolling is handled on the C++ side.
--
function message_view()

   --
   -- The current message
   --
   msg = current_message()

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
-- Read input, and evaluate it.
--
function read_eval()
   local txt = Screen:get_line(":");
   loadstring( txt )()
end
--
-- Read a line of text and execute the result as a command
--
function read_execute()
   local cmd = Screen:get_line("!");
   os.execute(cmd)
end






--
-- 3. Define some navigational-functions.
--
-----------------------------------------------------------------------------


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
-- 4. Define some call-backs which are implemented at various times.
--
-----------------------------------------------------------------------------


--
-- This function is called to generate tab-completions
--
function on_complete( token )

   --
   -- Some fixed things that we should be able to complete upon.
   --
   tmp = {
      "Panel:height",
      "Panel:hide",
      "Panel:show",
      "Panel:text",
      "Panel:title",
      "Panel:toggle",
      "Panel:visible",
      "Screen:clear",
      "Screen:exit",
      "Screen:get_line",
      "Screen:height",
      "Screen:maildir",
      "Screen:message",
      "Screen:sleep",
      "Screen:width",
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
-- Update the status-bar on-idle.
--
function on_idle()
--   Panel:title( "$[RED]Red title here. This must be serious!" )
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
-- 5. Define our key-bindings.
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
keymap['global']['M']   = "change_mode( 'maildir' );"
keymap['global']['I']   = "change_mode( 'index' );"
keymap['global']['L']   = "change_mode( 'lua' );"


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
keymap['maildir']['q'] = "os.exit()"
keymap['index']['q']   = "change_mode('maildir')"
keymap['message']['q'] = "change_mode('index')"


--
-- Enter/Leave attachment-mode
--
keymap['message']['A']    = "change_mode( 'attachment' );"
keymap['attachment']['q'] = "change_mode( 'message' );"






--
-- 6. Configure the mail-client.
--
-----------------------------------------------------------------------------

--
-- Setup the prefix to our maildir hierarchy.
--
Config:set( "maildir.prefix", os.getenv( "HOME" ) .. "/Maildir" )

--
-- Setup our default editor.  Not used at this time.
--
Config:set( "global.editor", "vim  +/^$ ++1 '+set tw=72'" )

--
-- Save persistant history of our input in the named file.
--
Config:set( "global.history", os.getenv( "HOME" ) .. "/.lumail2.history" )



--
-- 7.  Handle any command-line argumenst
--
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
   -- TODO: See why this doesn't work.  Outside the amin loop?
   --
   local txt = string.match(arg, "--eval=(.*)" )
   if ( txt ) then
      Panel:title(txt)
      f = loadstring( txt )
      f()
   end
end
