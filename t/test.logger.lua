--
-- Configure a sane load-path
--
package.path = package.path .. ";t/?.lua;../lib/?.lua;lib/?.lua"

--
-- Require our unit-testing framework.
--
luaunit = require 'luaunit'


--
-- File-helper
--
TestLogger = {}


--
-- Test with no level set.
--
function TestLogger:test_no_log ()

  -- Ensure there is no log set
  Config:set("log.path", nil)
  Log:level ""

  luaunit.assertEquals(Log:level(), "")
end


--
-- Test with a level set to `all`.
--
function TestLogger:test_log_all ()

  -- Create a temporary file
  local tmp = os.tmpname()
  luaunit.assertEquals(File:exists(tmp), true)

  -- Delete it
  os.remove(tmp)
  luaunit.assertEquals(File:exists(tmp), false)

  -- Use it
  Config:set("log.path", tmp)
  Log:level "all"

  Log:log("moi", "I like cakes")

  --
  -- The file should now exist
  --
  luaunit.assertEquals(File:exists(tmp), true)

  --
  -- Read the file; look for our line
  --
  local found = false
  for line in io.lines(tmp) do
    if string.match(line, "I like cakes") then
      found = true
    end
  end
  luaunit.assertEquals(found, true)

  -- Cleanup
  os.remove(tmp)
end


--
-- Test with a level set to `all`.
--
function TestLogger:test_log_levels ()

  -- Create a temporary file
  local tmp = os.tmpname()
  luaunit.assertEquals(File:exists(tmp), true)

  -- Delete it
  os.remove(tmp)
  luaunit.assertEquals(File:exists(tmp), false)

  -- Use it
  Config:set("log.path", tmp)
  Log:level "moi"

  --
  -- Now we should only find updates if the level is set to `moi`
  --
  local s = File:stat(tmp)
  Log:log("fail", "fail")
  local e = File:stat(tmp)

  -- So that line didn't match and our entries should be the same
  luaunit.assertEquals(s, e)

  -- Now log anew
  Log:log("moi", "This is present")
  local b = File:stat(tmp)
  luaunit.assertNotEquals(s, b)

  luaunit.assertTrue(b['size'] > s['size'])

  -- Unset the level
  Log:level ""
  Config:set("log.path", "")

  -- So this will fail
  Log:log("moi", "This is not logged")

  -- And our log-file won't change.
  local e = File:stat(tmp)
  luaunit.assertEquals(e, b)

  -- Cleanup
  os.remove(tmp)
  luaunit.assertEquals(File:exists(tmp), false)
end

--
-- Run the tests
--
os.exit(luaunit.LuaUnit.run())
