--
-- The keymap table.
--
-- There is a sub-table for each valid mode.
--
keymap = {}
keymap['global'] = {}

--
-- Setup a table for each registered mode
--
for i, o in ipairs(Global:modes()) do
  keymap[o] = {}
end

--
-- Global keybindings, which work in all modes.
--
keymap['global'][':'] = "read_eval()"
keymap['global']['!'] = "read_execute()"
keymap['global']['TAB'] = "Panel:toggle()"
keymap['global']['M'] = "change_mode( 'maildir' )"
keymap['global']['I'] = "change_mode( 'index' )"
keymap['global']['L'] = "change_mode( 'lua' )"
keymap['global']['^P'] = "change_mode( 'panel' )"
keymap['global']['^L'] = "change_mode( 'life' )"

--
-- Next/Previous navigation for different modes
--
keymap['global']['KEY_UP'] = "prev(false)"
keymap['global']['k'] = "prev(false)"
keymap['global']['j'] = "next(false)"
keymap['global']['KEY_DOWN'] = "next(false)"

keymap['global']['K'] = "prev(true)"
keymap['global']['KEY_PPAGE'] = "prev(true)"
keymap['global']['J'] = "next(true)"
keymap['global']['KEY_NPAGE'] = "next(true)"

keymap['global']['<'] = "first()"
keymap['global']['KEY_HOME'] = "first()"
keymap['global']['*'] = "last()"
keymap['global']['>'] = "last()"
keymap['global']['KEY_END'] = "last()"

keymap['global']['ENTER'] = "select()"
keymap['global']['SPACE'] = "select()"

--
-- Message-mode allows J/K to move to next/prev message
--
keymap['message']['J'] = 'next_message()'
keymap['message']['K'] = 'prev_message()'

--
-- Left/Right scrolling.  (Global)
--
keymap['global']['h'] = "left()"
keymap['global']['KEY_LEFT'] = "left()"
keymap['global']['l'] = "right()"
keymap['global']['KEY_RIGHT'] = "right()"

--
-- Forward/backward-searching (Global)
--
keymap['global']['/'] = 'find(1)'
keymap['global']['?'] = 'find(-1)'

--
-- Show keybindings: first of all global, then for a given key.
--
keymap['global']['H'] = "change_mode('keybinding')"
keymap['keybinding']['?'] = 'show_key_binding()'

--
-- Life
--
keymap['life']['r'] = 'life:clear(); life:random()'
keymap['life']['a'] = 'life:acorn()'
keymap['life']['c'] = 'life:clear()'
keymap['life']['g'] = 'life:gliders()'

--
-- Change the display-limits
--
keymap['maildir']['a'] = 'Config:set( "maildir.limit", "all" )'
keymap['maildir']['n'] = 'Config:set( "maildir.limit", "new" )'
keymap['maildir']['t'] = 'Config:set( "maildir.limit", "today" )'

--
-- Move to the next unread thing.
--
keymap['maildir']['N'] = 'next_unread()'
keymap['index']['N'] = 'next_unread()'


--
-- Limit the display of messages appropriately
--
keymap['index']['a'] = 'Config:set( "index.limit", "all" )'
keymap['index']['A'] = 'Config:set( "index.limit", "attach" )'
keymap['index']['n'] = 'Config:set( "index.limit", "new" )'
keymap['index']['t'] = 'Config:set( "index.limit", "today" )'

--
-- Change the sorting method.
--
-- Commented these out by default for two reasons:
--
--  1. Using `A` for `index.limit` is nice and memorable.
--
--  2. I don't believe users change their sorting methods on the fly.
--
--keymap['index']['A'] = 'sorting_method( "file")'
--keymap['index']['S'] = 'sorting_method( "subject")'
--keymap['index']['D'] = 'sorting_method( "date")'
--keymap['index']['F'] = 'sorting_method( "from")'
--

--
-- Toggle a message from new to old, or back.
--
keymap['index']['T'] = 'Message.toggle()'

--
-- Exit out of modes, and the application.
--
keymap['global']['q'] = "previous_mode()"
keymap['global']['Q'] = "os.exit(0)"

--
-- Enter attachment-mode
--
keymap['message']['A'] = "change_mode('attachment')"


--
-- Attachment-mode
--
-- Save the current attachment
keymap['attachment']['s'] = "save_mime_part()"
-- View the current attachment.
keymap['attachment']['SPACE'] = "view_mime_part()"
keymap['attachment']['ENTER'] = "view_mime_part()"
-- View the attachment with the given browser
keymap['attachment']['l'] = "view_mime_part( 'lynx -force_html %s' )"
keymap['attachment']['w'] = "view_mime_part( 'w3m -T text/html %s' )"


--
-- Actions relating to messages.
--
keymap['global']['c'] = 'Message.compose()'
keymap['message']['r'] = 'Message.reply()'
keymap['index']['r'] = 'Message.reply()'
keymap['message']['R'] = 'Message.reply_all()'
keymap['index']['R'] = 'Message.reply_all()'
keymap['message']['f'] = 'Message.forward()'
keymap['index']['f'] = 'Message.forward()'
keymap['message']['d'] = 'Message.trash()'
keymap['index']['d'] = 'Message.trash()'
keymap['message']['^D'] = 'Message.delete()'
keymap['index']['^D'] = 'Message.delete()'
keymap['message']['s'] = 'Message.save()'
keymap['index']['s'] = 'Message.save()'

keymap['message']['SPACE'] = 'next(10)'

--
-- Toggle display of full maildir paths
--
keymap['maildir']['p'] = 'Config.toggle( "maildir.truncate" )'

--
-- Toggle the size of the panel
--
keymap['global']['P'] = 'panel_size_toggle()'

--
-- Toggle line-wrap
--
keymap['global']['w'] = 'Config.toggle( "line.wrap" )'

--
-- Toggle the display of full headers / all parts
--
keymap['message']['G'] = 'Config.toggle( "message.headers" )'
keymap['message']['T'] = 'Config.toggle( "message.all_parts" )'


--
-- Test that we could set limits via a prefix.
--
keymap['maildir']['.a'] = 'Config:set( "maildir.limit", "all" )'
keymap['maildir']['.n'] = 'Config:set( "maildir.limit", "new" )'
keymap['maildir']['.t'] = 'Config:set( "maildir.limit", "today" )'

keymap['index']['.a'] = 'Config:set( "index.limit", "all" )'
keymap['index']['.A'] = 'Config:set( "index.limit", "attach" )'
keymap['index']['.n'] = 'Config:set( "index.limit", "new" )'
keymap['index']['.t'] = 'Config:set( "index.limit", "today" )'


keymap['global']['^A'] = 'Panel:append( "^A" )'
keymap['global']['^A^B'] = 'Panel:append( "^A^B" )'
keymap['global']['^A^B^C'] = 'Panel:append( "^A^B^C" )'


--
-- Jump to the given line, by number.
--
-- 1000 should be enough for everybody.
--
for i = 1, 999 do
  keymap['global'][tostring(i)] = 'jump(' .. i .. ');'
end


--
-- Helper to lookup the binding of a given key, in the specified mode.
--
function lookup_key (mode, key)
  return keymap[mode][key]
end


return keymap
