
function show_file( filename )
   if ( File:exists( filename ) ) then
      print( "File exists: " .. filename )

      data = File:stat( filename )
      for k,v in pairs( data ) do
         print( "\t" .. k .. ": " .. v )
      end
   else
      print( "File not found: " .. filename )
   end
end

--
-- Show some files
--
show_file( "/etc/passwd" )
show_file( "/bin/ls" )



--
-- Now let us look at the filesystem beneath /tmp
--
entries = Directory:entries( "/tmp" )

for k,v in ipairs(entries) do
   data = File:stat( v )
   print( string.format( "%10s %s", data.type, v) )
end
