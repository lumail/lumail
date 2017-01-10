
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
  if not s then
    error "Format string must be non-nil"
  end

  local left_pad = true
  local char_pad = " "

  return (s:gsub('($%b{})', function (key)
      local orig = key

      -- Get the name of the table-key we'll lookup
      key = key:sub(3, -2)

      -- Does this key have a length/padding restriction?
      local len, name = string.match(key, '^([0-9]+)|(.*)$')
      local name2, len2 = string.match(key, '^(.*)|([0-9]+)$')

      if len2 and name2 then
        len = len2
        name = name2
        left_pad = false
      end

      -- If so
      if len and name then

        -- Get the value - as a string.
        local val = tostring(tab[name] or orig)

        -- Are we zero-padding instead of space-padding?
        if len:sub(0, 1) == "0" then
          char_pad = "0"
        end

        -- The length of the field.
        len = tonumber(len)

        -- If the value is too long, truncate.
        if val:len() > len then
          val = val:sub(0, len)
        else
          -- Otherwise pad.
          while val:len() < len do
            if left_pad then
              val = char_pad .. val
            else
              val = val .. char_pad
            end
          end
        end
        return val
      else
        return tab[key] or orig
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
  if not s then
    error "The supplied argument must be non-nil"
  end
  return string.match(s, '^()%s*$') and '' or string.match(s, '^%s*(.*%S)')
end


--
-- Split a string on newlines, and return the result as a table.
--
-- This is used in some drawing modes.
--
_G['string']['to_table'] = function (str)

  if not str then
    error "The supplied argument must be non-nil"
  end

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
  if (not str) or (not sep) then
    error "The supplied arguments must be non-nil"
  end

  local result = {}
  local regex = ("([^%s]+)"):format(sep)
  for each in str:gmatch(regex) do
    table.insert(result, each)
  end
  return result
end
