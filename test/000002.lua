--
-- Test case-insensitive access to headers works.
--


--
-- Load a message and get the headers.
--
input   = Message.new( "test/000002.input" )
headers = input:headers()

--
-- Test that we can find the header with name "FROM", "From", "frOM", etc.
--

local one = input:header( "From" )
if ( one == "bobby <example@example.com>" ) then
   print("OK 1 - Read 'From' header")
end

local two = input:header( "FROm" )
if ( two == "bobby <example@example.com>" ) then
   print("OK 2 - Read 'From' header")
end

local twa = input:header( "frOM" )
if ( twa == "bobby <example@example.com>" ) then
   print("OK 3 - Read 'From' header")
end

local cat = input:header( "FROM" )
if ( cat == "bobby <example@example.com>" ) then
   print("OK 4 - Read 'From' header")
end


if ( ( one == two ) and
     ( two == twa ) and
     ( one == cat ) ) then
   print( "OK 5 - Headers all have identical contents." )
end