--
-- Simple progress-bar implementation
--
-----------------------------------------------------------------------------
-----------------------------------------------------------------------------

local Progress = {}


--
-- Show progress
--
do

   --
   -- We're going to assume UTF-8-aware console.
   --
   local step_chars = { "←", "↖", "↑", "↗", "→", "↘", "↓", "↙" }
   local step_off   = 1
   local step_count = 0

   --
   -- Animate a progress bar
   --
   function Progress.show_percent(self, cur, max )
      --
      -- Empty string
      --
      local padding = ""
      while( #padding < Screen:width() ) do
         padding = padding .. " "
      end

      if ( cur >= max ) then
         Screen:draw(0,0, padding, 1 )
         return
      end
      --
      -- Percentage of completion?
      --
      local t = math.floor(max / Screen:width())
      local s = ""
      for i=1,Screen:width() do
         if ( cur >= ( t * i) ) then
            s  = s .. ">"
         else
            s = s .. " "
         end
      end
      Screen:draw(0,0, s, 1 )
   end

   --
   -- A simple "spinner" animation which updates every few hundred
   -- iterations.
   --
   function Progress.step( self, str )
      local padding = ""
      while( #padding < Screen:width() ) do
         padding = padding .. " "
      end

      if ( str == nil ) then
         Screen:draw(0,0,padding)
         return
      end

      step_count = step_count + 1
      Screen:draw(0,0, step_chars[step_off] .. " " .. str .. padding, 1 )

      -- Don't update every time.
      -- Number chosen at random
      if ( step_count >= 250 ) then
         step_off = step_off + 1;
         if ( step_off > #step_chars )then
            step_off = 1
         end
         step_count = 0
      end
   end
end


return Progress
