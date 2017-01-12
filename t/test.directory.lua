--
-- Configure a sane load-path
--
package.path = package.path .. ";t/?.lua;../lib/?.lua;lib/?.lua"

--
-- Require our unit-testing framework.
--
luaunit = require 'luaunit'


--
-- Config-helper
--
TestDirectory = {}

function TestDirectory:test_exists ()

  -- Create a temporary file, and unlink it
  local tmp = os.tmpname()
  os.remove(tmp)

  -- So this shouldn't exist.
  luaunit.assertEquals(Directory:exists(tmp), false)

  -- Now make it
  Directory:mkdir(tmp)
  luaunit.assertEquals(Directory:exists(tmp), true)

  -- Finally cleanup
  os.remove(tmp)
  luaunit.assertEquals(Directory:exists(tmp), false)
end


--
-- Create some files, and then count them.
--
function TestDirectory:test_entries ()

  -- Create a temporary file, and unlink it
  local tmp = os.tmpname()
  os.remove(tmp)

  -- Now make it as a directory
  Directory:mkdir(tmp)
  luaunit.assertEquals(Directory:exists(tmp), true)

  -- Create some files
  local files = {
    "bar",
    "baz",
    "foo",
    "kemp",
    "steve",

  }

  for _, name in ipairs(files) do
    local file = io.open(tmp .. "/" .. name, "w")
    file:write(name)
    file:close()
  end

  -- Read the entries in the tree
  local out = Directory:entries(tmp)

  -- Filter out "/.", "/..", and "/".
  local found = {}
  for i, o in ipairs(out) do
    local c = string.sub(o, #o)
    if (o ~= tmp) and (c ~= ".") then
      table.insert(found, File:basename(o))
    end
  end

  -- The contents match
  luaunit.assertItemsEquals(files, found)

  -- Finally cleanup
  for _, name in ipairs(files) do
    os.remove(tmp .. "/" .. name)
  end
  os.remove(tmp)
  luaunit.assertEquals(Directory:exists(tmp), false)
end


--
-- Run the tests
--
os.exit(luaunit.LuaUnit.run())
