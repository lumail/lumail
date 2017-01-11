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
FUN = require "functional"


--
-- Utility function(s)
--
function square (a)
  return (a * a)
end

function is_prime (n)
  for i = 2, n^(1 / 2) do
    if (n % i) == 0 then
      return false
    end
  end
  return true
end

--
-- Test the functional.map function
--
TestMap = {}

function TestMap:test_basics ()
  luaunit.assertIsFunction(FUN.map)

  luaunit.assertErrorMsgContains("must be a function", FUN.map, nil)

  -- Our starting table.
  local input = {
    1,
    2,
    3,
    4,
    5,



  }

  -- The same table, squared.
  local output = FUN.map(square, input)

  -- For each entry we should have the input and output
  -- having the correct relationship
  for i, o in ipairs(input) do
    local given = input[i]
    local found = output[i]

    luaunit.assertEquals(given * given, found)
  end
end


--
-- Test the functional.object_map function
--
TestObjectMap = {}

function TestObjectMap:test_length ()
  luaunit.assertIsFunction(FUN.object_map)
  luaunit.assertErrorMsgContains("must be a string", FUN.object_map, nil)

  local input = {
    "one",
    "two",
    "three",
    "four",
    "five",



  }
  local output = FUN.object_map("len", input)

  --
  -- The size of the output will be the same as the input
  --
  luaunit.assertEquals(#input, #output)

  --
  -- For each input the expected result is the length of that string
  --
  for i, o in ipairs(input) do
    luaunit.assertEquals(string.len(input[i]), output[i])
  end

  --
  -- Now try reversing
  --
  output = FUN.object_map("reverse", input)

  --
  -- The size of the output will be the same as the input
  --
  luaunit.assertEquals(#input, #output)

  --
  -- The output will match, in reverse.
  --
  for i, o in ipairs(input) do
    luaunit.assertEquals(string.reverse(input[i]), output[i])
  end
end


--
-- Test the functional.filter function
--
TestFilter = {}

function TestFilter:test_prime ()
  luaunit.assertIsFunction(FUN.filter)
  luaunit.assertErrorMsgContains("must be a function", FUN.filter, nil)

  local input = {
    1,
    2,
    3,
    4,
    5,
    6,
    7,
    8,
    9,
    10,
    11,
    12,
    13,
    14,
    15,
    16,
    17,
    18,
    19,
    20,



  }
  local output = FUN.filter(is_prime, input)

  --
  -- The size of the output will be eleven
  --
  luaunit.assertEquals(#input, 20)
  luaunit.assertEquals(#output, 11)

  -- None of the output values will be prime.
  for i, o in ipairs(output) do
    luaunit.assertEquals(is_prime(o), false)
  end
end


--
-- Run the tests
--
os.exit(luaunit.LuaUnit.run())
