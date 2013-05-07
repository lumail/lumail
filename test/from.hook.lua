--
--  Test we can do something sensible with folder-hooks.
--
function get_from( folder )

   if ( string.find( folder, "steve.org.uk" , 1, true )  ) then
      from="steve@steve.org.uk"
   elseif ( string.find( folder, "debian-administration.org", 1, true )  ) then
      from="steve@debian-administration.org"
   elseif ( string.find( folder, "debian" , 1, true )  ) then
      from="skx@debian.org"
   end
   return from
end

--
-- Print the result from the function and the expected
--
print( "Result", "Expected" );
print( get_from( "/home/skx/Maildir/.debian-administration.org/" ) , "steve@debian-administration.org" )
print( get_from( "/home/skx/Maildir/.debian.org/" ) , "skx@debian.org" )
print( get_from( "/home/skx/Maildir/.steve.org.uk/" ) , "steve@steve.org.uk" )