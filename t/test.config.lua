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
TestConfig = {}

function TestConfig:test_get_set ()

  -- Get/Set - Unset by default
  luaunit.assertEquals(Config:get "foo", nil)

  -- Set
  Config:set("foo", "bar")

  -- Now it works
  luaunit.assertEquals(Config:get "foo", "bar")
  luaunit.assertIsString(Config:get "foo")
end


--
-- Test our known-good types
--
function TestConfig:test_types ()

  -- number
  Config:set("foo", 3)
  luaunit.assertEquals(Config:get "foo", 3)
  luaunit.assertIsNumber(Config:get "foo")

  -- string
  Config:set("foo", "bart")
  luaunit.assertEquals(Config:get "foo", "bart")
  luaunit.assertIsString(Config:get "foo")

  -- table
  local tbl = {
    "Kirsi",
    "Steve",
    "Oiva",



  }
  Config:set("family", tbl)
  luaunit.assertEquals(Config:get "family", {
      "Kirsi",
      "Steve",
      "Oiva",



    })
  luaunit.assertIsTable(Config:get "family")
  luaunit.assertEquals(#tbl + 1, #Config:get "foo")

  -- bool can be set - but becomes 0/1
  Config:set("truth", true)
  luaunit.assertEquals(Config:get "truth", 1)
  luaunit.assertIsNumber(Config:get "truth")

  -- bool can be set - but becomes 0/1
  Config:set("truth", false)
  luaunit.assertEquals(Config:get "truth", 0)
  luaunit.assertIsNumber(Config:get "truth")
end

--
-- Test that setting increases the key-count
--
function TestConfig:test_keys ()

  -- We get the table of keys - lumail will setup some defaults
  local one = Config:keys()
  luaunit.assertIsTable(one)

  -- Set a new key
  Config:set("moi", "kissa")
  local two = Config:keys()
  luaunit.assertIsTable(two)

  -- The new key increases our count
  luaunit.assertEquals(#one + 1, #two)
end

--
-- Run the tests
--
os.exit(luaunit.LuaUnit.run())
