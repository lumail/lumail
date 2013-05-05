--
-- lumail.lua - Init file for lumail.
--


--
-- The single mandatory setting is the prefix for the
-- maildir hierarchy.
--
set_maildir( os.getenv( "HOME" ) .. "/Maildir" );


--
-- This function is called when the client launches.
--
-- You might use something like this, usefully:
--
--    os.execute( "imapsync ..." );
--
function on_start()
   msg("I've started" );
end


--
-- This function is called when the client exits.
--
-- It could be used to do many things..
--
function on_exit()
   print("print: on_exit");
   io.write( "io.write: on_exit\n")
   io.write("The global mode is: '" .. get_mode() .. "'.\n");
end


--
-- This function is called when the client is idle.
--
-- You might run something like:
--
--   if the current-time is a multiple of five-minutes
--     os.execute("imapsync ..")
--
-- You might prefer to call fetchmail, or similar.
--
function on_idle()
   msg( "I am the lua function 'on_idle' - " .. os.date() .. "\n")
end


--
-- Switch to the index-view mode.
--
function index()
   set_mode( "index" );
   redraw_display();
end

--
-- Switch to the maildir-mode.
--
function maildir()
   set_mode( "maildir" );
   redraw_display();
end


--
-- Now setup keymaps for the different modes.
--
-- The same keypress might do different things in different modes,
-- which is one of the advantages of a modal-client.
--
--
--  TODO: Should there be a global-keymap?  For example:
--
--   keymap['q'] = 'exit()';
--
keymap = {}
keymap['index']   = {}
keymap['message'] = {}
keymap['maildir'] = {}

--
-- globally exit
--
keymap['index']['q']   = "exit()"
keymap['message']['q'] = "exit()"
keymap['maildir']['q'] = "exit()"

--
-- REPL-like-mode.
--
keymap['index'][':']   = "loadstring(prompt(\":\"))();";
keymap['message'][':'] = "loadstring(prompt(\":\"))();";
keymap['maildir'][':'] = "loadstring(prompt(\":\"))();";

--
-- Move to Maildir-mode
--
keymap['index']['M']   = 'maildir()';
keymap['message']['M'] = 'maildir()';
keymap['maildir']['M'] = 'maildir()';

--
-- Move to Index-mode
--
keymap['index']['I']   = 'index()';
keymap['message']['I'] = 'index()';
keymap['maildir']['I'] = 'index()';