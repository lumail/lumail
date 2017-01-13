--
-- Configure a sane load-path
--
package.path = package.path .. ";t/?.lua;../lib/?.lua;lib/?.lua"

--
-- Require our unit-testing framework.
--
luaunit = require 'luaunit'

--
-- Helper: Trivial HEX decoder
--
function string.fromhex(str)
    return (str:gsub('..', function (cc)
        return string.char(tonumber(cc, 16))
    end))
end

--
-- Helper: Get the MIME type from the given content.
--
function get_mime_type_bytes( content )
   -- make a temporary file.
   local tmp = os.tmpname()
   luaunit.assertEquals(File:exists(tmp), true)

   -- write the content
   local file = io.open(tmp, "w")
   file:write(content)
   file:close()

   -- Get the type
   local mime = MIME.type(tmp)

   -- cleanup
   os.remove(tmp)
   luaunit.assertEquals(File:exists(tmp), false)

   return(mime)
end

--
-- Basic testing
--
function testMime ()
   luaunit.assertIsFunction(MIME.type)

   -- Nil will return nil.
   luaunit.assertEquals( MIME.type(nil), nil )

   local sh = get_mime_type_bytes("#!/bin/sh\necho ok\n")
   luaunit.assertEquals( sh, "text/x-shellscript")

   local gif = get_mime_type_bytes( "GIF87a" )
   luaunit.assertEquals( gif, "image/gif")

   local png = get_mime_type_bytes( string.fromhex("89504E470D0A1A0A000000000000000000000000000000000000000000000000000000000") )
   luaunit.assertEquals( png, "image/png")

   local avi = get_mime_type_bytes( string.fromhex(  "524946460102030441564920"))
   luaunit.assertEquals( avi, "video/x-msvideo")

end


--
-- Run the tests
--
os.exit(luaunit.LuaUnit.run())
