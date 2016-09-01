--
-- A general "key = value" cache which persists to disk.
--
-- The user may setup a prefix for the cache-file on disk, but the name
-- of the file will always be the version of lumail2 which is running.
--
-- This allows us to run with both a release and a git-checkout, creating
-- files such as:
--
--    ~/.lumail2/cache/release-2.7
--    ~/.lumail2/cache/64fd-dirty
--
----
-----
----
--
-- This object can be used as follows:
--
--    Cache = require( "cache" )
--
--    local c = Cache.new()
--    c:load()
--
--    c:set( "foo", "bar" )
--    c:set( "foo", "bart" )
--    print( "Fetched from cache 'foo' -> " .. c:get( "foo" ) )
--    c:save()
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
-- Set a cache value, with a path and a key, not just a key.
--
function Cache.set_file( self, file, name, value )
   local key = file .. "'" .. name
   self:set( key, value )
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
-- Get a cached value, with a path and a key, not just a key, if it
-- exists
--
function Cache.get_file( self, file, name )
   local key = file .. "'" .. name
   return(self:get( key ))
end


--
-- Load our cache from disk.
--
function Cache.load( self )
   --
   -- The user must setup a cache prefix.
   --
   local dir  = Config:get( "cache.prefix" )
   if ( not dir ) then
      Panel:append( "$[RED]WARNING:$[WHITE] cache.prefix is not set" )
      return
   end

   --
   -- The cache file itself is the prefix plus the version
   --
   local file = dir .. "/" .. Config:get( "global.version" )

   if (file) and File:exists( file ) then
      local s = os.time()
      dofile( file )
      local e = os.time()
      Panel:append( "$[RED]INFO:$[WHITE] Cache load took " .. ( e - s ) .. " seconds" )
   else
      Panel:append( "$[RED]WARNING:$[WHITE] No cache present" )
   end
end

--
-- Save our cache.
--
function Cache.save(self)

   --
   -- Get the cache-prefix
   --
   local dir = Config:get( "cache.prefix" )
   if ( not dir ) then
      return
   end

   --
   -- Ensure the directory exists
   --
   if ( not Directory:exists( dir ) ) then
      Directory:mkdir( dir )
   end

   --
   -- Now write there
   --
   local file = dir .. "/" .. Config:get( "global.version" )

   if (file) then
      local hand = io.open(file,"w")

      -- Now the key/values from our cache.
      for key,val in pairs(self.store) do

         if ( type(key) ~= "string" ) then
            key = tostring(key)
         end
         key = key:gsub( "\"", "\\\"" )

         if ( type(val) ~= "string" ) then
            val = tostring(val)
         end
         val = val:gsub( "\"", "\\\"" )

         hand:write( "CacheEntry{ key = \"" .. key .. "\", val=\"" .. val .. "\"}\n" );
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
