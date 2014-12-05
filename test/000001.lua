--
--  Test that we can read an "accented" character in a header.
--

input   = Message.new( "test/000001.input" )
headers = input:headers()

--
-- Get the contents of the "From:" header
--
from = headers[ "from" ]

--
-- Test that it is correct.
--
if ( from == "Raphaël Rigo <example@example.com>" ) then
   print( "OK. We decoded the From-header." )
end