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
TestFile = {}


--
-- Test path-manipulation
--
function TestFile:test_basename ()
  local tests = {
    ["/etc/passwd"] = "passwd",
    ["/etc/motd"] = "motd",
    ["/etc/////motd"] = "motd",
    ["/foo"] = "foo",

  }

  -- Run all the tests
  for inp, outp in pairs(tests) do
    luaunit.assertEquals(File:basename(inp), outp)
  end
end


--
-- File existance
--
function TestFile:test_exists ()

  -- Create a temporary file
  local tmp = os.tmpname()
  luaunit.assertEquals(File:exists(tmp), true)

  -- Remove it and test it is gone
  os.remove(tmp)
  luaunit.assertEquals(File:exists(tmp), false)
end


--
-- File copying
--
function TestFile:test_copy ()
  -- Create a temporary input-file
  local inp = os.tmpname()
  luaunit.assertEquals(File:exists(inp), true)

  -- Create a temporary output file
  local outp = os.tmpname()
  luaunit.assertEquals(File:exists(outp), true)
  os.remove(outp)
  luaunit.assertEquals(File:exists(outp), false)

  -- Now write some known-content to the source
  local file = io.open(inp, "w")
  local text = os.time() .. "\n"
  file:write(text)
  file:close()

  -- Do the copy
  File:copy(inp, outp)

  -- Did it work?
  luaunit.assertEquals(File:exists(inp), true)
  luaunit.assertEquals(File:exists(outp), true)

  -- Do the files match?
  local src = File:stat(inp)
  local dst = File:stat(outp)
  luaunit.assertEquals(src["size"], dst["size"])

  -- Could test contents here.  skipped.
  os.remove(inp)
  os.remove(outp)
  luaunit.assertEquals(File:exists(inp), false)
  luaunit.assertEquals(File:exists(outp), false)
end


function TestFile:test_stat ()

  -- Create a temporary file, and unlink it
  local tmp = os.tmpname()
  os.remove(tmp)

  -- Should fail a stat
  local s = File:stat(tmp)
  luaunit.assertEquals(s, nil)

  -- Now create it
  local file = io.open(tmp, "w")
  local text = "0123456789\n"
  file:write(text)
  file:close()

  -- Re-stat
  s = File:stat(tmp)

  -- This will work
  luaunit.assertIsTable(s)
  luaunit.assertEquals(s["size"], #text)

  -- Test that the time of the file is "recent"
  local now = os.time()
  local crt = s["ctime"]
  luaunit.assertIsNumber(crt)

  -- Test we're sensible and that the file was created less than five
  -- seconds ago.
  local diff = math.abs(now - crt)
  luaunit.assertTrue(diff < 5)

  -- All done
  os.remove(tmp)
  luaunit.assertEquals(File:exists(tmp), false)
end

--
-- Run the tests
--
os.exit(luaunit.LuaUnit.run())
