--
-- Configure a sane load-path
--
package.path = package.path .. ";t/?.lua;../lib/?.lua;lib/?.lua"

--
-- Require our unit-testing framework.
--
luaunit = require 'luaunit'


--
-- Test that Message:flags works well.
--
TestMessageFlags = {}

function TestMessageFlags:test_blah ()

  -- Create a message (ha!)
  local tmp = os.tmpname()

  local msg = Message.new(tmp)

  -- Get the flags
  local start = msg:flags()

  -- The flags are empty
  luaunit.assertEquals(start, "")

  -- Mark the message as read
  msg:mark_read()

  -- Get the flags
  local ended = msg:flags()

  -- The flags will now have changed,
  -- the flags will be (S)een.
  luaunit.assertNotEquals(start, ended)
  luaunit.assertEquals(ended, 'S')

  -- Now mark the message as unread
  msg:mark_unread()

  -- Get the flags
  local final = msg:flags()

  -- The flags will now have changed, again.
  luaunit.assertNotEquals(ended, final)
  luaunit.assertEquals(final, '')

  -- Find the updated path
  os.remove(msg:path())
end


--
-- Run the tests
--
os.exit(luaunit.LuaUnit.run())
