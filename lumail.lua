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
   io.write("The global mode is: '" .. get_mode() .. "'.\n");
end

function on_idle()
   msg( "I am the lua function 'on_idle' - " .. os.date() .. "\n")
end


function index()
   set_mode( "index" );
   redraw_display();
end

function maildir()
   set_mode( "maildir" );
   redraw_display();
end

function message()
   set_mode( "message" );
   redraw_display();
end


keymap = {}
keymap['index']   = {}
keymap['message'] = {}
keymap['maildir'] = {}

-- globally exit
keymap['index']['Q']   = "exit()"
keymap['message']['Q'] = "exit()"
keymap['maildir']['Q'] = "exit()"
