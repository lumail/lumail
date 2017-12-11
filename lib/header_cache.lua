--
-- Implementation of a simple header cache for lumail.
--
-- Copyright (c) 2017 by Florian Fischer. All rights reserved.
--
-- This program is free software; you can redistribute it and/or modify
-- it under the terms of the GNU General Public License as published by
-- the Free Software Foundation; version 2 dated June, 1991, or (at your
-- option) any later version.
--
-- The license text is included in the LICENSE file at the root of the project.
--
--
-- Design:
-- ===
--
-- The cache is a normal lua table with the message path as a key and a table
-- holding the header pairs of the message as value.
--
-- The cache table is stored on disk in this format:
--
-- path/to/msg
-- foo: bar
-- bar: foo
--
-- path/to/secondmsg
-- different: header
-- ...
--
-- Header folding is not supported because it is only needed by smtp. 
-- And we receive our headers already parsed from gmime.
--
--
-- Locations:
-- ===
--
-- Each maildir has its own cache table. And every cache table is separately stored
-- on disk. The cache of a maildir is stored under the same relative path as the maildir.
--
-- E.g.:
-- maildir location: maildir.prefix/INBOX
-- cache location:   cache.prefix/INBOX/cache
--


-- Scope to keep the our actual cache variable private.
do
  -- Handle to load and store the header cache
  local Hcache = {}

  --
  -- The actual cache
  --
  local _cache = {}

  --
  -- Overwrite Message:header to make use of the header cache
  --
  do
    local __header = Message.header
    Message.header = function(msg, field)

      local entry = _cache[msg:path()]
      -- message is not cached load it. This only happens in still uncached maildirs.
      if not entry then
        entry = msg:headers()
        _cache[msg:path()] = entry
      end

      -- CMessage stores the header names in lower case
      return entry[field:lower()] or ""
    end
  end

  -- Helper to calculate the correct cache file location
  local function cache_location()
    local prefix = Config:get("cache.prefix")
    if prefix == nil then
      return nil
    end

    -- If there is no current_maildir we can't load its header cache
    local maildir = Global:current_maildir()
    if not maildir then
      return nil
    end

    local maildir_location = maildir:path()
    local suffix = maildir_location:match("^" .. Config:get("maildir.prefix") .. "(.*)")

    return prefix .. "/hcache" .. suffix
  end
  
  --
  --- Load the header cache from file.
  --
  -- This should be called when opening a maildir
  --
  Hcache.load = function()
    local location = cache_location()
    if location == nil then
      return
    end

    Log:log("hcache", "Loading from : " .. location .. "/cache")

    -- Empty cache
    _cache = {}

    local f = io.open(location .. "/cache", "r")

    if f == nil then
      return
    end

    -- Parse each header block
    local expect_path = true
    local entry = {}
    for l in f:lines() do
      if expect_path then
        entry = {}
        _cache[l] = entry
        expect_path = false
      elseif l == "" then
        expect_path = true
      else
        local key, value = l:match("^(.-): (.*)")
        entry[key:lower()] = value
      end
    end

    f:close()
  end

  --
  --- Save the header cache to file.
  --
  -- This should be called when leaving a maildir.
  --
  Hcache.save = function()
    local location = cache_location()
    if location == nil then
      return
    end

    Log:log("hcache", "Saving to: " .. location .. "/cache")

    -- Create the cache location if it not exists
    if not Directory:exists(location) then
      Log:log("hcache", "Create cache location: " .. location)
      Directory:mkdir(location)
    end

    local f = io.open(location .. "/cache", "w")

    if f == nil then
      return
    end

    for path, headers in pairs(_cache) do
      f:write(path .. "\n")
      for h, v in pairs(headers) do
        f:write(h .. ": " .. v .. "\n")
      end
      f:write("\n")
    end

    f:close()
  end

  return Hcache
end
