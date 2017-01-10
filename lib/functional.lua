--
-- Simple functional-programming examples
--

local Functional = {}


--
--  Call the given function on all members of the table.
--
function Functional.map (func, array)
  if ( type(func) ~= "function" ) then
     error("The first argument must be a function" )
  end
  local new_array = {}
  local i = #array
  while i > 0 do
    new_array[i] = func(array[i])
    i = i - 1
  end
  return new_array
end

--
-- For every object in the given array call "object.function(object)"
--
-- NOTE:
--   self:method() is a shortcut for self.method(self), and in turn
--   self.method is syntactic sugar for self['method'].
--
-- Example
--   If `m` is a Message object we can call `mark_read()` by invoking
--
--      m['mark_read'](m)
--
function Functional.object_map (func, array)
   if ( type(func) ~= "string" ) then
      error("Function must be a string" )
   end
  local new_array = {}
  local i = #array
  while i > 0 do
    local v = array[i]
    new_array[i] = v[func](v)
    i = i - 1
  end
  return new_array
end

--
-- Return an array of the given elements, except for those
-- who return true from the given function.
--
function Functional.filter (func, arr)
  if ( type(func) ~= "function" ) then
     error("The first argument must be a function" )
  end
  local new_array = {}
  for i, v in ipairs(arr) do
    if not func(v) then
      table.insert(new_array, v)
    end
  end
  return new_array
end

--
-- Return our object.
--
return Functional
