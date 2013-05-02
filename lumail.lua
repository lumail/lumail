-- the first program in every language
io.write("Hello world, from lua version ",_VERSION, " lumail version ", VERSION, "!\n")

--
-- Test getting/setting the current mode.
--
set_mode( "index" );
io.write("The global mode is " .. get_mode() .. "\n");

a = my_function( 1, 4, 9 );
io.write("my_function() returned " .. a .. "\n");


function on_idle()
   io.write( "I am the lua function 'on_idle'.\n")
end
