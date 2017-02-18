--
-- Configure a sane load-path
--
package.path = package.path .. ";t/?.lua;../lib/?.lua;lib/?.lua"

--
-- Require our unit-testing framework.
--
luaunit = require 'luaunit'


--
--  Holder.
--
TestCache = {}

--
-- Basic testing
--
function TestCache:test_functions ()

  --
  -- The expected functions should exist.
  --
  luaunit.assertIsFunction(Cache.new)
  luaunit.assertIsFunction(Cache.load)
  luaunit.assertIsFunction(Cache.save)
  luaunit.assertIsFunction(Cache.get)
  luaunit.assertIsFunction(Cache.set)
  luaunit.assertIsFunction(Cache.empty)
end


--
-- Load/Save should do something useful.
--
function TestCache:test_save_load ()

  --
  -- Create a temporary file-name
  --
  local tmp = os.tmpname()
  os.remove(tmp)
  luaunit.assertEquals(File:exists(tmp), false)

  --
  -- Create a cache object, populate it, and save it.
  --
  local c = Cache.new()
  c:set("foo", "bar")
  c:save(tmp)

  --
  --  Now the file should exist
  --
  luaunit.assertEquals(File:exists(tmp), true)


  --
  -- Create a second cache object, so we can test
  -- that loading worked
  --
  local n = Cache.new()
  luaunit.assertEquals(n:get "foo", nil)

  --
  -- Post-Load we should have the value
  --
  n:load(tmp)
  luaunit.assertEquals(n:get "foo", "bar")

  --
  -- All done
  --
  os.remove(tmp)
end


--
-- Emptying a cache should work.
--
function TestCache:test_cache_empty ()

  --
  -- Create a temporary file-name
  --
  local tmp = os.tmpname()
  os.remove(tmp)
  luaunit.assertEquals(File:exists(tmp), false)

  --
  -- Create a cache object, populate it, and save it.
  --
  local c = Cache.new()
  c:set("foo", "bar")
  c:set("moi", "kiss")
  c:set("steve", "kemp")
  c:save(tmp)

  --
  --  Now the file should exist, and be non-zero in size
  --
  local stat = File:stat(tmp)
  luaunit.assertTrue(stat['size'] > 0)
  luaunit.assertEquals(File:exists(tmp), true)


  --
  -- Empty the cache and resave
  --
  c:empty()
  c:save(tmp)

  --
  -- The file should still exist, but it should be empty
  --
  stat = File:stat(tmp)
  luaunit.assertTrue(stat['size'] == 0)
  luaunit.assertEquals(File:exists(tmp), true)

  --
  -- All done
  --
  os.remove(tmp)
end

--
-- Run the tests
--
os.exit(luaunit.LuaUnit.run())
