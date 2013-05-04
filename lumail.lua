-- the first program in every language
io.write("Hello world, from lua version ",_VERSION, " lumail version ", VERSION, "!\n")

--
-- Test getting/setting the current mode.
--
set_mode( "maildir" );
io.write("The global mode is " .. get_mode() .. "\n");

function on_start()
   msg("I've started" );
end

function on_exit()
   print("print: on_exit");
   io.write( "io.write: on_exit\n")
end

function on_idle()
   msg( "I am the lua function 'on_idle' - " .. os.date() .. "\n")
end
