--
-- lumail.lua - Init file for lumail.
--


--
-- The single mandatory setting is the prefix for the
-- maildir hierarchy.
--
maildir_prefix( os.getenv( "HOME" ) .. "/Maildir" );

--
-- Show all folders by default
--
-- When called wiht no arguments this function will return the current
-- setting, otherwise it will update it.
--
-- The maildir limit can be set to three values:
--
--    all  -> Show all maildir folders.
--    new  -> Show all maildir folders which contain unread messages.
--   "str" -> Show all maildir folders which contain the substring "str" in their paths.
--
-- See "all" , "new", and "livejournal" functions for example of use.
--
maildir_limit( "all" );



--
-- The index format controls how messages are displayed inside folder lists.
--
-- Valid options are sadly limited to:
--
--   $DATE
--   $FLAGS
--   $FROM
--   $SUBJECT
--   $TO
--
-- TODO: Make the date more selectable..
--
index_format( "[$FLAGS] $FROM - $SUBJECT" );

--
-- This function is called when the client is launched.
--
-- You might consider something useful like this:
--
--    os.execute( "imapsync ..." );
--
function on_start()
   msg("lumail v" .. VERSION .. " http://lumail.org/" );
end


--
-- This function is called when a folder is selected.
--
-- It is where you can run per-folder hooks.
--
-- Remember: More than one folder might be selected.
--
function on_select_folder( f )

   --
   -- This is test-code.
   --
   -- ff = io.open( "/tmp/log.lua", "a");
   -- ff:write( "Opened " .. f .. "\n" );
   -- ff:close();

end


--
-- Show the version of this client.
--
function show_version()
   msg("This is lumail version " .. VERSION );
end


--
-- This function is called when the client exits.
--
-- It could also be used to run an imap-sync, or similar hook at exit-time.
--
function on_exit()
   print("print: on_exit");
   io.write( "io.write: on_exit\n")
   io.write("The global mode is: '" .. global_mode() .. "'.\n");
   io.write("The limit is " .. maildir_limit() .. "\n" );
end


--
-- This function is called when the client is idle.
--
-- You might run something like:
--
--   if the current-time is a multiple of five-minutes
--     os.execute("imapsync ..")
--
-- (Of course you may prefer to call fetchmail, or similar.)
--
function on_idle()
   m = global_mode()
   m = string.lower( m );

   str = ""

   if ( string.find( m, "maildir" ) ) then
      str = "mode:" .. m ..  " limit:" .. maildir_limit();
   else
      str = "mode:" .. m ;
   end

   msg( str .. " time:" .. os.date("%X" ) );
end


--
-- Switch to the index-view mode.
--
-- This will display the messages contained in all currently-selected folders.
--
-- (Selected folders will be displayed with a "[x]" next to them in maildir-mode.)
--
function index()
   global_mode( "index" );
   clear();
end


--
-- Switch to the maildir-mode.
--
function maildir()
   global_mode( "maildir" );
   clear();
end


--
-- When in maildir-mode show all folders.
--
function all_folders()
   maildir_limit( "all" );
   clear();
end


--
-- When in maildir-mode show all folders which contain unread messages.
--
function new_folders()
   maildir_limit( "new" );
   clear();
end


--
-- When in maildir-mode show all folders which have a path matching the given pattern.
--
function livejournal_folders()
   maildir_limit( "livejournal.2" );
   clear();
end


--
-- Navigation functions for maildir-mode
--
function maildir_down()
   scroll_maildir_down( 1 );
end
function maildir_page_down()
   scroll_maildir_down( 10 );
end
function maildir_up()
   scroll_maildir_up(1);
end
function maildir_page_up()
   scroll_maildir_up(10);
end



--
-- Navigation functions for index-mode
--
function message_down()
   scroll_index_down( 1 );
end
function message_page_down()
   scroll_index_down( 10 );
end
function message_up()
   scroll_index_up(1);
end
function message_page_up()
   scroll_index_up(10);
end


--
-- Search for the next folder/message which matches the entered pattern.
--
function search_next()
   x = prompt("/:" );
   if ( string.find( m, "maildir" ) ) then
      scroll_maildir_to( x );
   else
      scroll_index_to( x );
   end
end


--
-- Open a folder exclusively.
--
-- This behaves like the traditional "open folder" function in our favourite
-- client.  I like to open multiple folders at once, because that's how I roll.
--
function open_folder()
   clear_selected_folders();
   add_selected_folder()
   global_mode( "index" );
   clear();
end



--
-- Open my most important folders.
--
function faves()
   global_mode( "index" );

   -- ensure all folders are available
   maildir_limit( "all" );
   clear_selected_folders();

   -- The first folder.
   scroll_maildir_to( "/.steve.org.uk" );
   add_selected_folder()

   -- The second folder.
   scroll_maildir_to( "/.people.kirsi" );
   add_selected_folder()

   -- Now we've selected two folders, open them.
   global_mode( "index" );
   clear();
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
-- Globally compose a new message.
--
keymap['index']['c']   = "compose()"
keymap['message']['c'] = "compose()"
keymap['maildir']['c'] = "compose()"

--
-- REPL-like-mode.
--
keymap['index'][':']   = "loadstring(prompt(\":\"))();";
keymap['message'][':'] = "loadstring(prompt(\":\"))();";
keymap['maildir'][':'] = "loadstring(prompt(\":\"))();";

--
-- Execute a shell command.
--
keymap['index']['!']   = "exec(prompt(\":\"));";
keymap['message']['!'] = "exec(prompt(\":\"));";
keymap['maildir']['!'] = "exec(prompt(\":\"));";

--
-- Show the client version
--
keymap['maildir']['v'] = 'show_version()'
keymap['index']['v']   = 'show_version()'
keymap['message']['v'] = 'show_version()'

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



--
-- Toggle display of folders
--
keymap['maildir']['a'] = 'all_folders()'
keymap['maildir']['n'] = 'new_folders()'
keymap['maildir']['l'] = 'livejournal_folders()'

--
-- Scroll up/down & find folders
--
keymap['maildir']['j'] = 'maildir_down()'
keymap['maildir']['J'] = 'maildir_page_down()'
keymap['maildir']['k'] = 'maildir_up()'
keymap['maildir']['K'] = 'maildir_page_up()'
keymap['maildir']['/'] = 'search_next()'
keymap['maildir']['f'] = 'faves()'


--
-- Scroll up/down the message index
--
keymap['index']['j'] = 'message_down()'
keymap['index']['J'] = 'message_page_down()'
keymap['index']['k'] = 'message_up()'
keymap['index']['K'] = 'message_page_up()'
keymap['index']['/'] = 'search_next()'

--
-- Selection bindings.
--
--  "space" will toggle the current folder as selected.
--  "return" will open the selected folder exclusively.
--
keymap['maildir'][' '] = 'toggle_selected_folder()'
keymap['maildir']['\n'] = 'open_folder()'
