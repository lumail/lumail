
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
-- Setup KeyMaps
--
keymap = {}
keymap['global']  = {}
keymap['demo']    = {}

--
-- Global Mode
--
keymap['global'][':'] = "read_eval()"
keymap['global']['\t'] = "Panel:toggle()"



--
-- Demo Mode
--
keymap['demo']['d' ] = "date()"
keymap['demo']['h' ] = "hostname()"
