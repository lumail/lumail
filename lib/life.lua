--
-- Simple implementation of life, in pure Lua.
--
-----------------------------------------------------------------------------
-----------------------------------------------------------------------------

--
-- Object setup
--
Life = {}
local mt = {
  __index = Life,

}


--
-- Copy the object we're given.
--
function deepcopy (object)
  local lookup_table = {}
  local function _copy (object)
    if type(object) ~= "table" then
      return object
    elseif lookup_table[object] then
      return lookup_table[object]
    end
    local new_table = {}
    lookup_table[object] = new_table
    for index, value in pairs(object) do
      new_table[_copy(index)] = _copy(value)
    end
    return setmetatable(new_table, getmetatable(object))
  end
  return _copy(object)
end

--
-- Clear the display
--
function Life:clear ()
  for i = 1, self.m do
    local row = {}
    for j = 1, self.n do
      row[j] = 0
    end
    self.matrix[i] = row
  end
end


--
-- Acorn setup
--
function Life:acorn ()
  local r_y = math.floor(Screen:height() / 2) - 4
  local r_x = math.floor(Screen:width() / 2)

  self.matrix[r_x][r_y] = 1
  self.matrix[r_x][r_y + 1] = 1
  self.matrix[r_x - 2][r_y + 1] = 1
  self.matrix[r_x - 1][r_y + 3] = 1
  self.matrix[r_x][r_y + 4] = 1
  self.matrix[r_x][r_y + 5] = 1
  self.matrix[r_x][r_y + 6] = 1
end

--
-- Random setup
--
function Life:random ()
  -- Fullness?
  local ful = math.random(16)

  local max = (self.m * self.n) / ful
  for i = 1, max do
    local r_x = math.random(Screen:width())
    local r_y = math.random(Screen:height())

    if (r_x > 0) and (r_x < (self.m - 2)) and (r_y > 0) and (r_y < (self.n - 2)) then
      self.matrix[r_x][r_y] = 1
    end
  end
end

--
-- Glider Setup
--
function Life:gliders ()

  -- Count?
  local count = math.random(32)

  for i = 1, count do
    local r_x = math.random(self.m)
    local r_y = math.random(self.n)

    -- Horrid
    if (r_x > 0) and (r_x < (self.m - 2)) and (r_y > 0) and (r_y < (self.n - 2)) then

      self.matrix[r_x][r_y] = 1
      self.matrix[r_x + 1][r_y] = 1
      self.matrix[r_x + 2][r_y] = 1
      self.matrix[r_x + 1][r_y + 2] = 1
      self.matrix[r_x + 2][r_y + 1] = 1
    end
  end
end

--
-- Constructor
--

function Life.new (m, n)
  local matrix = {}

  for i = 1, m do
    local row = {}
    for j = 1, n do
      row[j] = 0
    end
    matrix[i] = row
  end


  return setmetatable({
      matrix = matrix,
      m = m,
      n = n,

    }, mt)
end


--
-- Run the next generation.
--
function Life:next_gen ()
  local X = deepcopy(self.matrix)
  local matrix = self.matrix
  for i = 1, self.m do
    for j = 1, self.n do
      local s = 0
      for p = i - 1, i + 1 do
        for q = j - 1, j + 1 do
          if p > 0 and p <= self.m and q > 0 and q <= self.n then
            s = s + self.matrix[p][q]
          end
        end
      end
      s = s - self.matrix[i][j]
      if s == 3 or (s + self.matrix[i][j]) == 3 then
        X[i][j] = 1
      else
        X[i][j] = 0
      end
    end
  end
  self.matrix = deepcopy(X)
end

--
-- Show the display
--
do
  local x = 0
  local step = 1

  function Life:print_matrix ()
    local symbols = {
      "■",
      "▣",
      "●",
      "▦",
      "▩",
      "▪",
      "□",

    }
    local colours = {
      "$[GREEN]",
      "$[RED]",
      "$[YELLOW]",
      "$[BLUE]",
      "$[MAGENTA]",
      "$[CYAN]",
      "$[RED]",

    }

    local day = os.date "*t".wday
    local today = symbols[math.min(day, #symbols)]
    local colour = colours[math.min(day, #colours)]

    for i = 1, self.m do
      for j = 1, self.n do
        if self.matrix[i][j] == 0 then
          Screen:draw(i, j, " ")
        else
          Screen:draw(i, j, colour .. today)
        end
      end
    end

    --
    -- Scrolling Banner
    --
    Screen:draw(x, 2, "$[GREEN]L$[BLUE]i$[RED]f$[YELLOW]e")

    x = x + step
    if x + 4 > Screen:width() then
      -- 4 is the length of the banner
      step = -1
    end
    if x < 0 then
      step = 1
    end
  end
end

--
-- Return the object.
--
return Life
