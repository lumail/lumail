--
-- Utility for counting the size of a table
--
_G['table']['size'] = function( obj )
   local i = 0
   for k,v in pairs (obj) do
      i = i + 1
   end
   return i
end

--
-- Utility for sorting table by keys
--
_G['table']['sorted_keys'] = function( tbl )
   local keys = {}
   for key in pairs(tbl) do
      table.insert(keys, key)
   end
   table.sort(keys, function(a, b) return ( a:lower() < b:lower() ) end)
   return keys
end
