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
-- Helper function - return a message with the given content
--
function txt2msg(txt)

   --
   -- Create a temporary file, and write our message to it
   --
   local tmp = os.tmpname()
   local file = io.open(tmp, "w")
   file:write(txt)
   file:close()


   --
   -- Create the object
   --
   local msg = Message.new(tmp)
   luaunit.assertEquals( tmp, msg:path() )

   return(msg)
end



--
-- Holder
--
TestMessage = {}


--
-- Basic testing
--
function TestMessage:test_functions ()
  --
  -- The functions we're using should exist.
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
   -- Get a new message object
   --
   local msg = txt2msg(msg_txt)

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
   os.remove(msg:path())
   luaunit.assertEquals(File:exists(msg:path()), false)
end



function TestMessage:test_mime_parts ()
   local msg_txt = [[Return-path: <leigh@chiashi.jp>
Envelope-to: steve@steve.example.com
Delivery-date: Mon, 16 Jan 2017 08:03:57 +0000
Received: from qpsmtpd (helo=ssh.example.com)
	by ssh.example with local-bsmtp (Exim 4.84_2)
	(envelope-from <leigh@chiashi.jp>)
	id 1cT2Gi-00080c-S5
	for steve@example.net; Mon, 16 Jan 2017 08:03:56 +0000
Received: from bt.ptr128.ptrcloud.net (HELO adadserver.jp) (153.122.74.46)
    by mail.example.net (qpsmtpd/0.84) with (ECDHE-RSA-AES256-GCM-SHA384 encrypted) ESMTPS; Mon, 16 Jan 2017 08:03:56 +0000
To: steve@steve.org.uk
Subject: ADV: Our profile is on top, looking for lover
X-PHP-Originating-Script: 10001:global.php(1955) : eval()'d code
Date: Mon, 16 Jan 2017 15:48:54 +0900
From: Leigh <leigh@chiashi.jp>
Message-ID: <d4348056771a78e01f94c58edf5c25af@chiashi.jp> (sfid-20170116_080357_075978_B8B91804)
X-Priority: 3
MIME-Version: 1.0
Content-Type: multipart/alternative;
	boundary="b1_d4348056771a78e01f94c58edf5c25af"
Content-Transfer-Encoding: 8bit

--b1_d4348056771a78e01f94c58edf5c25af
Content-Type: text/plain; charset=us-ascii

Two pretty student girls ready to make you smile


--b1_d4348056771a78e01f94c58edf5c25af
Content-Type: text/html; charset=us-ascii

<html>
<body>
Two pretty student girls ready to make you smile.
</body>
</html>



--b1_d4348056771a78e01f94c58edf5c25af--
]]


   --
   -- Get a new message object
   --
   local msg = txt2msg(msg_txt)

   --
   -- We're gonna test the message-parts
   --
   local parts = msg:parts()

   --
   -- There will be one part `multipart/alternative` with
   -- a child part for `text/html` and `text/plain`.
   --
   luaunit.assertEquals( #parts, 1 )
   luaunit.assertEquals( parts[1]:type(), "multipart/alternative" )

   --
   -- Get those two child-parts
   --
   local c = parts[1]:children()
   luaunit.assertEquals( #c, 2 )

   --
   -- Test they have the distinct types
   --
   for i,o in ipairs(c) do
      luaunit.assertTrue( ( o:type() == "text/html" ) or
                          ( o:type() == "text/plain" ) )
   end

   --
   -- Just for completeness the two types will differ - i.e we
   -- have two parts, which are either text/html or text/plain
   -- but they will be one of each
   --
   luaunit.assertTrue(c[1]:type() ~= c[2]:type())

   --
   -- Cleanup
   --
   os.remove(msg:path())
   luaunit.assertEquals(File:exists(msg:path()), false)
end


--
-- Test removal
--
function TestMessage:test_unlink ()
   local msg_txt = [[To: steve@example.net
From: steve@example.com
Subject: This is my subject
Date: Mon Jan 16 09:56:15 EET 2017

This is a message, from email, honest.

Steve
--
              ]]


   --
   -- Get a new message object
   --
   local msg = txt2msg(msg_txt)

   --
   -- Test it exists - implied
   --
   luaunit.assertEquals(File:exists(msg:path()), true)

   --
   -- Remove
   --
   msg:unlink()

   --
   -- Confirm it is gone
   --
   luaunit.assertEquals(File:exists(msg:path()), false)

end


--
-- Run the tests
--
os.exit(luaunit.LuaUnit.run())
