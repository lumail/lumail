--
-- Test we can identify new messages.
--
--
-- A message is new if:
--
--   1.  It has the "N" flag.
--
-- or
--
--   2.  It does not have the "S" flag.
--



input = Message.new( "test/1417028782.21867_3.ssh.steve.org.uk:2,S" )
flags = input:flags()

if ( flags == "S" ) then
   print( "OK 1 - Read flags from message path." )
end


--
-- Now we have an N-flag
--
input = Message.new( "test/new/1417028782.21867_3.ssh.steve.org.uk:2" )
flags = input:flags()

if ( flags == "N" ) then
   print( "OK 2 - Read new-flag from message path." )
end


--
-- Finally we have a flagged message
--
input = Message.new( "test/new/1417028782.21867_3.ssh.steve.org.uk:2,SNF" )
flags = input:flags()

if ( flags == "FNS" ) then
   print( "OK 3 - Read new-flags from message path." )
end
