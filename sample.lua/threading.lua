--
-- Print the threaded and sorted INBOX and print information about missing or
-- overridden messages.
--

--
-- Recursively print one thread.
--
local messages_in_tree = {}

function print_tree (self, indent)
  if self.message then
    -- collect messages in the tree to compare with msgs
    messages_in_tree[self.message] = 1
    local date = string.gsub(self.message:header "Date", ".*(%d%d)%s(%a%a%a)%s%d%d%d%d%s(%d%d:%d%d).*", "%2 %1 %3")
    print(indent .. "> " .. date .. " " .. self.message:header "Subject")
  else
    print(indent .. "> Empty Container")
  end
  for i, v in ipairs(self.children) do
    print_tree(v, indent .. "-")
  end
end

local Threader = require "threader"

local folders = Global:maildirs()
local maildir = nil

for i, v in ipairs(folders) do
  if v:path() == Config:get "maildir.prefix" .. "/INBOX" then
    maildir = v
    break
  end
end

Global:select_maildir(maildir)
local msgs = Global:current_messages()

local roots = Threader.thread(msgs)
local roots = Threader.sort(roots, _G["compare_by_date"], "max", false)

-- Print threads and collect all threaded messages.
for k, v in pairs(roots) do
  --v:get
  print_tree(v, "")
  print()
end
print("Threads: " .. #roots)

-- Handle missing messages.
local missing = 0
local overridden = 0
for i, v in ipairs(msgs) do
  if not messages_in_tree[v] then
    -- we know that we loose those messages
    if not Threader.overridden[v] then
      print("Missing message subject: " .. v:header "Subject")
      missing = missing + 1
    else
      overridden = overridden + 1
    end
  end
end
print("Overridden messages: " .. overridden)
print("Missing messages: " .. missing)
