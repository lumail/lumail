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
-- Flush all known keys - i.e. empty the cache.
--
function Cache.flush( self )
   self.store = nil
   self.store = {}
end

--
-- Set a value in the cache.
--
function Cache.set( self, name, value )
   self.store[name] = value
end


--
-- Get the size of our cache.
--
function Cache.size( self )
   local i = 0
   for k,v in pairs (self.store) do
      i = i + 1
   end
   return i
end


--
-- Get a value from the cache, if it exists.
--
function Cache.get( self, name )
   return( self.store[name] )
end

--
-- Load our cache from disk.  If it is too large empty it
-- afterwards to avoid excessive size.
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
-- Save our cache - trim it if it is too large first.
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
-- Flush our cache if it is "too large".
--
function Cache.trim(self)
   local size = self:size()
   if ( size and ( size > 50000 ) ) then
      self:flush()
   end
end

return Cache
