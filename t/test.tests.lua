--
-- Test that all the functions defined in t/*.lua match the pattern
-- "test" - which means they will be executed by luaunit.
--
-- Filter out some known-good exceptions (mocked functions).
--
----


--
-- Configure a sane load-path
--
package.path = package.path .. ";t/?.lua;../lib/?.lua;lib/?.lua"

--
-- Require our unit-testing framework.
--
luaunit = require 'luaunit'

--
-- We'll use string.trim
--
SU = require "string_utilities"


--
--  Return file contents as a table.
--
function int_read_file_contents (file)
  lines = {}
  for line in io.lines(file) do
    lines[#lines + 1] = line
  end
  return lines
end


--
--  Test a single file
--
function int_test_single_file (file)
  --
  -- Read the file and look for functions
  --
  local lines = int_read_file_contents(file)

  --
  -- For each line in the file
  --
  for i, o in ipairs(lines) do

    --
    -- Does this line define a function?
    --
    local name = string.match(o, "^function(.*)%(.*%)")

    --
    -- We strip out some names which are known-good
    --
    -- (These are mocked functions, or things to simplify the test-cases)
    --
    local ok = {
      "Cache:new",
      "Config:get",
      "File:exists",
      "Global:modes",
      "Net:hostname",
      "Panel:append",
      "Screen:draw",
      "Screen:exit",
      "Screen:height",
      "Screen:width",
      "file_size",
      "fmt",
      "get_mime_type_bytes",
      "int_read_file_contents",
      "is_prime",
      "os.execute",
      "square",
      "string.fromhex",
      "string.path",
      "txt2msg",

    }

    --
    --  If we have a function name
    --
    if name then

      --
      -- Remove the leading/trailing whitespace.
      --
      name = string.trim(name)

      --
      -- Strip out the names that are OK.
      --
      for i, o in ipairs(ok) do
        if name == o then
          name = nil
        end
      end

      --
      -- The function must match "test" if we have a name
      -- (i.e. we didn't rule it out)
      --
      if name then
        luaunit.assertNotIsNil(string.match(name, "[Tt][eE][sS][tT]"))
      end
    end
  end
end

--
-- Test all files in a directory
--
function int_test_directory (dir)

  -- Get each directory entry.
  local ent = Directory:entries(dir)
  for i, o in ipairs(ent) do

    -- If it is a *.lua file
    if string.match(o, "%.lua") then

      -- But not luaunit
      if not string.match(o, "unit") then

        -- Test the functions defined within.
        int_test_single_file(o)
      end
    end
  end
end

--
-- Look for tests
--
function test_tests ()
  luaunit.assertEquals(Directory:exists "t/", true)
  int_test_directory "t/"
end


--
-- Run the tests
--
os.exit(luaunit.LuaUnit.run())
