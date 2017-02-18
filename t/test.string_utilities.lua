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
SU = require "string_utilities"


--
-- Setup some data.
--
local test_data = {
  forename = "Apu",
  pi = math.pi,
  surname = "Nahasapeemapetilon",



}


--
-- Define a helper function
--
function fmt (fmt)
  return (string.interp(fmt, test_data))
end


--
-- Test the string.interp function
--
TestInterp = {}

function TestInterp:testFunction ()
  luaunit.assertIsFunction(string.interp)
end

function TestInterp:testBasic ()
  luaunit.assertEquals(fmt "${forename}", "Apu")
  luaunit.assertEquals(fmt "${forename} ${forename}", "Apu Apu")
  luaunit.assertEquals(fmt "${surname}", "Nahasapeemapetilon")
  luaunit.assertEquals(fmt "${forename} ${surname}", "Apu Nahasapeemapetilon")
  luaunit.assertEquals(fmt "${surname} ${forename}", "Nahasapeemapetilon Apu")

  --
  -- We expect a string as a result, not a number.
  --
  luaunit.assertIsString(fmt "${pi}")
end

function TestInterp:testFailures ()
  luaunit.assertEquals(fmt "Test", "Test")
  luaunit.assertEquals(fmt "${Test}", "${Test}")
  luaunit.assertEquals(fmt "${}", "${}")
  luaunit.assertEquals(fmt "", "")
  luaunit.assertErrorMsgContains("Format string must be non-nil", fmt, nil)
end

function TestInterp:testSpacePadding ()
  luaunit.assertEquals(fmt "${10|forename}", "       Apu")
  luaunit.assertEquals(fmt "${2|surname}", "Na")

  luaunit.assertEquals(fmt "${09|forename}", "000000Apu")
  luaunit.assertEquals(fmt "${02|surname}", "Na")
end

function TestInterp:testRightPadding ()
  luaunit.assertEquals(fmt "${forename|10}", "Apu       ")
  luaunit.assertEquals(fmt "${forename|010}", "Apu0000000")
  luaunit.assertEquals(fmt "${surname|2}", "Na")
  luaunit.assertEquals(fmt "${surname|02}", "Na")
end

--
-- Test our UTF-aware (ha!) string.len method.
--
function TestInterp:test_len ()
   local a = "Muller"
   luaunit.assertEquals( a:len(), 6)
   luaunit.assertEquals( a:len(), old_len(a))

   local b = "Müller"
   luaunit.assertEquals( b:len(), 6)
   luaunit.assertEquals( old_len(b), 7)

   local c = "«ÜßÄ$€»"
   luaunit.assertEquals( c:len(), 7)
   luaunit.assertEquals( old_len(c), 14)

   local d = "‹ÅåÄäÖö›"
   luaunit.assertEquals( d:len(), 8)
   luaunit.assertEquals( old_len(d), 18)
end


--
-- Test the string.split function
--
TestSplit = {}

function TestSplit:testFunction ()
  luaunit.assertIsFunction(string.split)
end

function TestSplit:testSingle ()
  local out = string.split("TEST", "n")
  luaunit.assertEquals(#out, 1)
end

function TestSplit:testMultiple ()
  local out = string.split("Test1\nTest2\nTest3", "\n")
  luaunit.assertEquals(#out, 3)

  -- Each match will be TestN where N is 1..3
  for i, o in pairs(out) do
    luaunit.assertEquals("Test" .. i, o)
  end
end

function TestSplit:testNil ()
  luaunit.assertErrorMsgContains("The supplied arguments must be non-nil", string.split, "Steve", nil)
  luaunit.assertErrorMsgContains("The supplied arguments must be non-nil", string.split, nil, "Steve")
  luaunit.assertErrorMsgContains("The supplied arguments must be non-nil", string.split, nil, nil)
end


--
-- Test the string.to_table function
--
TestToTable = {}

function TestToTable:testFunction ()
  luaunit.assertIsFunction(string.to_table)
end

function TestToTable:testSingle ()
  local out = string.to_table "TEST"
  luaunit.assertEquals(#out, 1)
end

function TestToTable:testMultiple ()
  local out = string.to_table "Test1\nTest2\nTest3"
  luaunit.assertEquals(#out, 3)
end

function TestToTable:testNil ()
  luaunit.assertErrorMsgContains("The supplied argument must be non-nil", string.to_table, nil)
end

--
-- Test the string.trim function
--
TestTrim = {}

function TestTrim:testFunction ()
  luaunit.assertIsFunction(string.trim)
end

function TestTrim:testLeading ()
  luaunit.assertEquals(string.trim " bob", "bob")
end

function TestTrim:testTrailing ()
  luaunit.assertEquals(string.trim "bob ", "bob")
end

function TestTrim:testMiddle ()
  luaunit.assertEquals(string.trim "bob o'tables", "bob o'tables")
end

function TestTrim:testAll ()
  luaunit.assertEquals(string.trim "  	bob	", "bob")
end

function TestTrim:testNil ()
  luaunit.assertErrorMsgContains("The supplied argument must be non-nil", string.trim, nil)
end

--
-- Run the tests
--
os.exit(luaunit.LuaUnit.run())
