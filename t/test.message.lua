--
-- Configure a sane load-path
--
package.path = package.path .. ";t/?.lua;../lib/?.lua;lib/?.lua"

--
-- Require our unit-testing framework.
--
luaunit = require 'luaunit'

--
-- We'll use `table.size` in our test.  Require the helper library.
--
TU = require "table_utilities"

--
-- Holder
--
TestMessage = {}


--
-- Basic testing
--
function TestMessage:test_functions ()
  --
  -- The stack functions should exist.
  --
  luaunit.assertIsFunction(Message.ctime)
  luaunit.assertIsFunction(Message.headers)
  luaunit.assertIsFunction(Message.header)
end


--
-- Simple header testing
--
function TestMessage:test_headers ()
   local msg_txt = [[To: steve@example.net
From: steve@example.com
Subject: This is my subject
Date: Mon Jan 16 09:56:15 EET 2017

This is a message, from email, honest.

Steve
--
              ]]

   --
   -- Create a temporary file, and write our message to it
   --
   local tmp = os.tmpname()
   local file = io.open(tmp, "w")
   file:write(msg_txt)
   file:close()


   --
   -- Create the object
   --
   local msg = Message.new(tmp)

   --
   -- Test our headers
   --
   luaunit.assertEquals( msg:header("From"), "steve@example.com" )
   luaunit.assertEquals( msg:header("FROM"), "steve@example.com" )

   luaunit.assertEquals( msg:header("To"), "steve@example.net" )
   luaunit.assertEquals( msg:header("TO"), "steve@example.net" )

   --
   -- Get all the headers - we should have four of them.
   --
   local h = msg:headers()
   luaunit.assertEquals( table.size(h), 4 )

   --
   -- Cleanup
   --
   os.remove(tmp)
   luaunit.assertEquals(File:exists(tmp), false)
end


--
-- Run the tests
--
os.exit(luaunit.LuaUnit.run())
