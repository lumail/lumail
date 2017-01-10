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
require "gpg"

--
-- Mocked function.
--
function string.path(binary)
--   print("XXXX:string.path(" .. binary .. ")")
   return true
end

--
-- Mocked function
--
function os.execute(cmd)
   --   print("XXXX:os.execute(" .. cmd .. ")")
   return true
end

--
-- Helper function
--
function file_size(filename)
   local file = io.open(filename,"r")
   local size = file:seek("end")
   file:close()
   return size
end


--
-- Test wrapper.
--
TestGPG = {}

function TestGPG:testFunction ()
   luaunit.assertIsFunction(message_replace)
end

function TestGPG:testEmptyFile ()

   filename = "test.gpg.lua.txt"
   --
   -- Open a file and write some lines - which don't have
   -- a PGP header
   --
   local file = io.open(filename, "w")
   file:write("This is a test\n")
   file:write("I like cakes\n" )
   file:close()

   local out = message_replace( filename )

   luaunit.assertEquals( out, "" )

   os.remove( filename )
end


function TestGPG:testPGPSignature ()

   filename = "test.gpg.lua.txt"
   --
   -- Open a file and write some lines - which don't have
   -- a PGP header
   --
   local file = io.open(filename, "w")
   file:write("-----BEGIN PGP SIGNATURE-----\n")
   file:write("I like cakes\n" )
   file:write("-----END PGP SIGNATURE-----\n")
   file:close()

   --
   -- At this point we'll have had a shell command executed
   -- which in the real system would have expanded the content.
   --
   -- The function returns a filename.  So we ensure that it
   -- even exists.
   --
   local out = message_replace( filename )
   luaunit.assertTrue( file_size(out) >= 0 )

   --
   -- Remove the file that we recieved - as well as our input
   --
   os.remove( out )
   os.remove( filename )
end


function TestGPG:testPGPMessage ()

   filename = "test.gpg.lua.txt"
   --
   -- Open a file and write some lines - which don't have
   -- a PGP header
   --
   local file = io.open(filename, "w")
   file:write("-----BEGIN PGP MESSAGE-----\n")
   file:write("I like cakes\n" )
   file:write("-----END PGP MESSAGE-----\n")
   file:close()

   --
   -- At this point we'll have had a shell command executed
   -- which in the real system would have expanded the content.
   --
   -- The function returns a filename.  So we ensure that it
   -- even exists.
   --
   local out = message_replace( filename )
   luaunit.assertTrue( file_size(out) >= 0 )

   --
   -- Remove the file that we recieved - as well as our input
   --
   os.remove( out )
   os.remove( filename )
end


--
-- Run the tests
--
os.exit(luaunit.LuaUnit.run())
