--
-- A general "key = value" cache which persists.
--
--
-- This object can be used as follows:
--
--    Cache = require( "cache" )
--
--    local c = Cache.new()
--    c:load( "t.logg")
--
--    c:set( "foo", "bar" )
--    c:set( "foo", "bart" )
--    print( "Fetched from cache 'foo' -> " .. c:get( "foo" ) )
--    c:save( "t.logg" )
--

local Cache = {}
Cache.__index = Cache


--
-- Constructor.
--
function Cache.new()
   local self = setmetatable({}, Cache)
   self.store = {}
   return self
end

--
-- Setter.
--
function Cache.set( self, name, value )
   self.store[name] = value
end

--
-- Getter.
--
function Cache.get( self, name )
   return( self.store[name] )
end

--
-- Loader
--
function Cache.load( self, file )
   if (file) and File:exists( file ) then

      -- our version
      local cur = "VERSION=" .. Config:get( "global.version" )
      -- The version from the cache-file.
      local ver = nil

      for line in io.lines(file) do

         -- If we've not got a version then the first line we see will be it.
         if ( not ver ) then
            ver = line
         else
            -- Does the version match our current release?
            if ( ver == cur ) then
               -- greedy match on key-name.
               key, val = line:match("^(.*)=([^=]+)$")
               if ( key and val ) then
                  self:set(key, val)
               end
            end
         end
      end
   end
end

--
-- Saver
--
function Cache.save(self, file)

   if (file) then
      local hand = io.open(file,"w")

      -- write out our version
      hand:write( "VERSION=" .. Config:get( "global.version" ) .. "\n" )

      -- Now the key/values from our cache.
      for key,val in pairs(self.store) do
         hand:write( key .. "=" .. val  .. "\n")
      end
      hand:close()
   end
end

--
-- TODO: Expire if too large
--


return Cache
