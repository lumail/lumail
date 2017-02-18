--
-- Configure a sane load-path
--
package.path = package.path .. ";t/?.lua;../lib/?.lua;lib/?.lua"

--
-- Require our unit-testing framework.
--
luaunit = require 'luaunit'

--
-- Holder
--
TestRegexp = {}


--
-- Basic testing
--
function TestRegexp:test_functions ()
  luaunit.assertIsFunction(Regexp.match)
end

function TestRegexp:test_basic ()

  --
  -- Test cases we expect to match
  --
  local passes = {
    ["Steve Kemp"] = "eve",
    ["1.2.3.4"] = "[0-9]+.[0-9]+.[0-9]+.[0-9]+",
    ["/home/steve/Maildir/people-chris/"] = "people-chr",

  }

  --
  -- Things we don't expect to match
  --
  local fails = {
    ["Steve Kemp"] = "^[^ ]+$",
    ["fe80::1"] = "[0-9]+.[0-9]+.[0-9]+.[0-9]+",

    -- % is the escape for Lua patterns, not PCRE.
    ["/home/steve/Maildir/people-chris/"] = "people%-chr",

  }

  for input, regexp in pairs(passes) do
    local res = Regexp:match(regexp, input)
    luaunit.assertTrue(res)
    luaunit.assertIsBoolean(res)
  end

  for input, regexp in pairs(fails) do
    local res = Regexp:match(regexp, input)
    luaunit.assertTrue(res == false)
    luaunit.assertIsBoolean(res)
  end
end

--
-- When a match contains captures the `match` function
-- will return a table, not a bool.
--
function TestRegexp:test_captures ()

  local ip = "172.172.168.1"
  local res = Regexp:match("^([0-9]+)\\.([0-9]+)\\.([0-9]+)\\.([0-9]+)$", ip)

  luaunit.assertIsTable(res)
  luaunit.assertEquals(#res, 4)

  luaunit.assertEquals(res[1], "172")
  luaunit.assertEquals(res[2], "172")
  luaunit.assertEquals(res[3], "168")
  luaunit.assertEquals(res[4], "1")
end


--
-- Run the tests
--
os.exit(luaunit.LuaUnit.run())
