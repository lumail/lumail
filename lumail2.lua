
--
-- Define some utility functions
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
-- Change the mode - and update the panel-title, if we have one.
--
function change_mode( new_mode )
   Config:set( "global.mode", new_mode )
   Panel:title( "Mode is now " .. new_mode .. " " )
end

--
-- Setup KeyMaps
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
