--
--
-- Usage:
--
--     ./lumail --load-file ./panel.lua
--
--
-- Once loaded execute the lua-prompt by typing ":" and enter
-- "date()" or "hostname()".
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


function date()
   show_command_output( "The date is", "date" );
end
function d()
   date()
end

function hostname()
   show_command_output( "The host is", "hostname" );
end
function h()
   hostname()
end
