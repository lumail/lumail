--
--
-- Usage:
--
--     lumail2 --load-file ./compose.lua --to=email@address.tld --subj=
--

-- Iterate over the command-line arguments
--
local recipient, subject
for i, v in ipairs(ARGS) do
  recipient = string.match(v, "--to=(.*)")
  subject = string.match(v, "--subj=(.*)")
end

Message.compose(recipient, subject)
os.exit(0)

