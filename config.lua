--
--
-- Usage:
--
--     ./lumail --no-curses --load-file ./config.lua
--




--
-- Test a string-value
--
function test_string()

   orig = Config:get( "steve" )
   if ( orig ) then
      print( "ERROR - Unset value is non-nil: " .. orig )
   else
      print( "Value is nil without setting it" )
   end

   Config:set( "steve", "kemp" )
   print( "Value is now set " .. Config:get( "steve" ) )
end


--
-- Test getting/setting a table.
--
function test_table()

   flowers = { "rose", "daisy", "tulip" }

   print( "Our table contains:")
   for k,v in ipairs( flowers ) do
      print( "\t" .. v )
   end

   --
   -- Set the table value
   --
   Config:set( "flowers", flowers )

   --
   -- Retrieve the values
   --
   out = Config:get( "flowers" )
   print( "After retrieving our table has:")
   for k,v in ipairs( out ) do
      print( "\t" .. v )
   end
end



test_table()
test_string()

--
-- I like a good pie.
--
Config:set( "π", "3.14159265359" )


--
-- Show all the keys we set
--
print( "\n")
for k,v in ipairs( Config:keys() ) do
   print( "We've set the configuration-key with name " .. v )

   val = Config:get( v )

   print( "\tThis has type '" .. type(val) .. "'" )

end
