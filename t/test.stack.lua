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
Stack = require "stack"


--
-- Basic testing
--
function testStack ()
  --
  -- The stack functions should exist.
  --
  luaunit.assertIsFunction(Stack.new)
  luaunit.assertIsFunction(Stack.push)
  luaunit.assertIsFunction(Stack.pop)
  luaunit.assertIsFunction(Stack.size)

  local s = Stack.new()

  -- Empty stack will only pop nil
  luaunit.assertEquals(s:pop(), nil)
  luaunit.assertEquals(s:pop(), nil)
  luaunit.assertEquals(s:pop(), nil)

  -- Add on numbers 1-10
  --
  -- The number should now be the size of the stack.
  --
  local i = 1
  while i < 11 do

    s:push(i)
    luaunit.assertEquals(s:size(), i)

    i = i + 1
  end

  -- Pop each number in tuurn
  i = 10
  while i > 0 do

    --
    -- We're popping in order, so we'll pop 10,9,8...
    --
    luaunit.assertEquals(s:pop(), i)

    --
    -- Each time we pop the size goes down by one, matching our index
    --
    luaunit.assertEquals(s:size(), i - 1)

    i = i - 1
  end

  --
  -- Stack should now be empty, and remain empty if more
  -- pops are attempted.
  --
  local i = 1
  while i < 11 do
    luaunit.assertEquals(s:size(), 0)
    s:pop()
    i = i + 1
  end

  --
  -- Finally the types of objects should remain the same
  --

  -- function
  s:push(math.min)
  luaunit.assertIsFunction(s:pop())

  -- string
  s:push "Steve"
  luaunit.assertIsString(s:pop())

  -- number
  s:push(math.pi)
  luaunit.assertIsNumber(s:pop())

  -- bool
  s:push(true)
  luaunit.assertIsBoolean(s:pop())

  -- no truncation?
  s:push(math.pi)
  luaunit.assertEquals(s:pop(), math.pi)
end


--
-- Run the tests
--
os.exit(luaunit.LuaUnit.run())
