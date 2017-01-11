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
Progress = require "progress_bar"

--
-- Mocked function.
--
Screen = {}
function Screen:width ()
  return 80
end

--
-- Mocked function
--
local line = ""
function Screen:draw (x, y, txt, refresh)
  line = txt
  return true
end


--
-- Test wrapper.
--
TestProgressBar = {}

function TestProgressBar:test_basics ()
  luaunit.assertIsFunction(Progress.show_percent)
  luaunit.assertIsFunction(Progress.step)
end

--
-- Screen:draw just updates a buffer with the contents of
-- the text we're supposed to have drawn.  Here we test that
-- we can retrieve what we drew.
--
function TestProgressBar:test_mocked_functions ()
  Screen:draw(0, 0, "steve", false)
  luaunit.assertEquals(line, "steve")

  Screen:draw(0, 0, "kemp", false)
  luaunit.assertEquals(line, "kemp")
end


--
-- Screen:draw just updates a buffer with the contents of
-- the text we're supposed to have drawn.  Here we test that
-- we can retrieve what we drew.
--
function TestProgressBar:test_show_percent ()
  -- Show 0% complete.
  Progress:show_percent(0, 100)

  -- That should just update our screen buffer with a string of
  -- whitespace 80 characters long - the length we set in Screen:width()
  luaunit.assertEquals(#line, Screen:width())

  --
  -- Test that the output was 80 spaces
  --
  local s80 = ""
  while #s80 < Screen:width() do
    s80 = s80 .. " "
  end
  luaunit.assertEquals(line, s80)

  --
  -- Same again at 100%
  --
  Progress:show_percent(99, 100)
  luaunit.assertEquals(#line, Screen:width())

  --
  -- Test that the output was 80 ">"s.
  --
  local w80 = ""
  while #w80 < Screen:width() do
    w80 = w80 .. ">"
  end
  luaunit.assertEquals(line, w80)


  --
  -- Finally 50%
  --
  Progress:show_percent(40, 80)
  luaunit.assertEquals(#line, Screen:width())

  local i = 0
  local space = 0
  local arrow = 0

  while i < (#line + 1) do
    if line:sub(i, i) == ' ' then
      space = space + 1
    end
    if line:sub(i, i) == '>' then
      arrow = arrow + 1
    end
    i = i + 1
  end

  -- Equal number of spaces and arrows - we're at 50%
  luaunit.assertEquals(space, arrow)

  -- And the total is half the screen
  luaunit.assertEquals(space, Screen:width() / 2)
  luaunit.assertEquals(arrow, Screen:width() / 2)
end

--
-- Screen:draw just updates a buffer with the contents of
-- the text we're supposed to have drawn.  Here we test that
-- we can retrieve what we drew, in the tick-case
--
function TestProgressBar:test_step ()

  -- If no string is given the ticker just draws a blank line.
  Progress:step()

  --
  -- Test that the output was 80 spaces
  --
  local s80 = ""
  while #s80 < Screen:width() do
    s80 = s80 .. " "
  end
  luaunit.assertEquals(line, s80)

  --
  -- Now we need to ensure that we receive some updates
  -- if we draw more than 500 times.
  --
  local start = line
  local cur = ""
  local update_count = 0

  local i = 0
  while i < 1000 do
    Progress:step "Progress .."
    if line ~= cur then
      update_count = update_count + 1
      cur = line
    end
    i = i + 1
  end

  --
  -- At this point we should have "stepped" more than
  -- once, so the current buffer will be different to the
  -- starting buffer
  --
  luaunit.assertNotEquals(line, start)

  --
  -- We know the ticker updates every 250 updates,
  -- so after 1000 steps we'll have seen four updates.
  --
  luaunit.assertTrue(update_count == 4)
end


--
-- Run the tests
--
os.exit(luaunit.LuaUnit.run())
