--
-- Configure a sane load-path
--
package.path = package.path .. ";t/?.lua;../lib/?.lua;lib/?.lua"

--
-- Require our unit-testing framework.
--
luaunit = require 'luaunit'

--
-- Require the library we're testing
--
TU = require "table_utilities"


--
-- Test the string.interp function
--
TestTableSize = {}

function TestTableSize:test_basics ()
  --
  luaunit.assertIsFunction(table.size)

  -- Empty table has size zero
  t = {}
  luaunit.assertEquals(table.size(t), 0)

  -- Now one.
  t["moi"] = "kissa"
  luaunit.assertEquals(table.size(t), 1)

  -- Now two.
  t[200] = 300
  luaunit.assertEquals(table.size(t), 2)

  -- We must supply a table.
  luaunit.assertErrorMsgContains("You must supply a table", table.size, nil)
  luaunit.assertErrorMsgContains("You must supply a table", table.size, "steve")
  luaunit.assertErrorMsgContains("You must supply a table", table.size, math.pi)
end


TestTableSorting = {}

function TestTableSorting:test_sorted_keys ()
  local input = {}
  input["z"] = "zzz"
  input["moi"] = "foo"
  input["a"] = "aaa"

  -- Ensure the function exists
  luaunit.assertIsFunction(table.sorted_keys)

  -- We must supply a table.
  luaunit.assertErrorMsgContains("You must supply a table", table.sorted_keys, nil)

  -- The sorted keys should now be : a, moi, z
  local out = table.sorted_keys(input)

  -- Test we got what we expect.
  luaunit.assertEquals(out[1], "a")
  luaunit.assertEquals(out[2], "moi")
  luaunit.assertEquals(out[3], "z")

  -- The size of the input and output should match.
  luaunit.assertEquals(table.size(out), table.size(input))
end


--
-- Run the tests
--
os.exit(luaunit.LuaUnit.run())
