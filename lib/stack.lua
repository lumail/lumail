--
-- Simple implementation of a stack, in pure Lua.
--
-----------------------------------------------------------------------------
-----------------------------------------------------------------------------

local Stack = {}
Stack.__index = Stack

function Stack.new ()
  local self = setmetatable({}, Stack)
  self.stack = {}
  return self
end
function Stack.push (self, value)
  local size = #self.stack
  self.stack[size + 1] = value
end
function Stack.size (self)
  local size = #self.stack
  return size
end
function Stack.pop (self)
  local size = #self.stack
  if size <= 0 then
    return nil
  end
  local value = self.stack[size]
  self.stack[size] = nil
  return value
end

return Stack
