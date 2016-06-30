--
-- Simple implementation of life, in pure Lua.
--
-----------------------------------------------------------------------------
-----------------------------------------------------------------------------

   Life = {}
   local mt = { __index = Life }


   function deepcopy(object)
      local lookup_table = {}
      local function _copy(object)
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


   function Life.new(m, n)
      local matrix = {}

      for i = 1, m do
         local row = {}
         for j = 1, n do
            row[j] = 0
         end
         matrix[i] = row
      end

      for i=1,50 do
         local r_x = math.random(m)
         local r_y = math.random(n)

         -- Horrid
         if ( ( r_x > 0 ) and
              ( r_x < ( m - 2) ) and
              ( r_y > 0 ) and
              ( r_y < ( n - 2) ) ) then

            matrix[r_x][r_y]     = 1
            matrix[r_x+1][r_y]   = 1
            matrix[r_x+2][r_y]   = 1
            matrix[r_x+1][r_y+2] = 1
            matrix[r_x+2][r_y+1] = 1
         end
      end

      return setmetatable({
                             matrix = matrix,
                             m = m,
                             n = n,
                          }, mt)
   end

   function Life:set_pos(x,y)
      self.matrix[x][y] = 1
   end

   function Life:unset_pos(x,y)
      self.matrix[x][y] = 0
   end

   function Life:next_gen()
      local X = deepcopy(self.matrix)
      local matrix = self.matrix
      for i = 1, self.m do
         for j = 1, self.n do
            local s = 0
            for p = i-1,i+1 do
               for q = j-1,j+1 do
                  if p > 0 and p <= self.m and q > 0 and q <= self.n then
                     s = s + self.matrix[p][q]
                  end
               end
            end
            s = s - self.matrix[i][j]
            if s == 3 or (s+self.matrix[i][j]) == 3 then
               X[i][j] = 1
            else
               X[i][j] = 0
            end
         end
      end
      self.matrix = deepcopy(X)
   end

   function Life:print_matrix()
      local matrix = self.matrix
      for i = 1, self.m do
         for j = 1, self.n do
            if matrix[i][j] == 0 then
               Screen:draw( i, j, " " )
            else
               Screen:draw( i, j, "*" )
            end
         end

      end
   end
   return Life
