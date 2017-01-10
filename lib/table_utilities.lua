--
-- Utility for counting the size of a table
--
_G['table']['size'] = function (obj)
  if type(obj) ~= "table" then
    error "You must supply a table"
  end

  local i = 0
  for k, v in pairs(obj) do
    i = i + 1
  end
  return i
end

--
-- Utility for sorting table by keys
--
_G['table']['sorted_keys'] = function (tbl)
  if type(tbl) ~= "table" then
    error "You must supply a table"
  end
  local keys = {}
  for key in pairs(tbl) do
    table.insert(keys, key)
  end
  table.sort(keys, function (a, b)
      return (a:lower() < b:lower())
    end)
  return keys
end

--
-- Utility for deleting entry in numeric part of a table
--
_G['table']['delete'] = function (table, obj)
  local found = false
  for i, v in ipairs(table) do
    if v == obj then
      found = true
    end

    if found then
      table[i] = table[i + 1]
    end
  end
end
