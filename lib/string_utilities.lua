
--
-- Does the specified string end with the given string?
--
_G['string']['ends'] = function (String, End)
   return End == '' or string.sub(String, -string.len(End)) == End
end


--
-- String interopolation function, taken from the Lua wiki:
--
--   http://lua-users.org/wiki/StringInterpolation
--
-- Usage:
--
--   print( string.interp( "Hello ${name}", { name = "World" } )
--
--
-- String interopolation function, based upon code from the Lua wiki:
--
--   http://lua-users.org/wiki/StringInterpolation
--
-- Usage:
--
--   print( string.interp( "Hello ${name}", { name = "World" } )
--
-- As an extension we can limit the size of expanded strings:
--
--   print( string.interp( "Hello ${10|name}",
--                        { name = "Apu Nahasapeemapetilon" } )
--
--  If the string is too long it will be truncted, and if too short will
-- be (left-)padded with spaces.
--
--
_G['string']['interp'] = function (s, tab)
   return (s:gsub('($%b{})', function (key)
                     -- Get the name of the table-key we'll lookup
                     key = key:sub(3,-2)

                     -- Does this key have a length restriction?
                     local len,name = string.match(key , '^([0-9]+)|(.*)$' )

                     -- If so
                     if ( len and name ) then
                        -- Get the value.
                        local val = tab[name] or name

                        -- Ensure our size is a number.
                        len = tonumber(len)

                        -- If the value is too long, truncate.
                        if ( val:len() > len ) then
                           val = val:sub(0,len)
                        else
                           -- Otherwise pad.
                           while( val:len() < len ) do
                              val = " " .. val
                           end
                        end
                        return val
                     else
                        return tab[key] or key
                     end
                             end))
end

--
-- Find the given binary on the $PATH, and return the full filename.
--
_G['string']['path'] = function (file)
   local path = os.getenv "PATH"
   local paths = string.split(path, ":;")
   for i, o in pairs(paths) do
      if File:exists(o .. "/" .. file) then
         return (o .. "/" .. file)
      end
   end
   return ""
end


--
-- Strip leading/trailing whitespace from the given string.
--
_G['string']['trim'] = function (s)
   return string.match(s, '^()%s*$') and '' or string.match(s, '^%s*(.*%S)')
end


--
-- Split a string on newlines, and return the result as a table.
--
-- This is used in some drawing modes.
--
_G['string']['to_table'] = function (str)
   local t = {}
   local function helper (line)
      table.insert(t, line)
      return ""
   end
   helper((str:gsub("(.-)\r?\n", helper)))
   return t
end


--
-- Split a string by a separator character, and return the resulting
-- table.
--
_G['string']['split'] = function (str, sep)
   local result = {}
   local regex = ("([^%s]+)"):format(sep)
   for each in str:gmatch(regex) do
      table.insert(result, each)
   end
   return result
end
