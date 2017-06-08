--
-- This is a function which is inserted into the *MIDDLE*
-- of a message parsing operation.
--
-- This means you CANNOT call `parts()` or similar, on a constructed
-- message, because if you do you'll end up in a never-ending recursive
-- loop.
--
-- The only thing you can do is *replace* the contents of the body
-- if you wish.
--
-- This function allows the caller to modify the message _before_ it
-- is parsed, and the way this is done is to replace the file that
-- is operated upon.
--
-- In short :
--
--  * This function is called with the path to the message, on-disk.
--
--  * This function may return "" to leave things as-is.
--
-- OR
--
--  * THis function may generate and return a filename.  That file
--    will be operated upon instead of the input message, and once
--    parsed will be deleted.
--
--
-- So if you wanted to do something crazy, like read only upper-case
-- messages, you could do that by running:
--
--   function message_replace(path)
--     local out = "/tmp/blah"
--     os.execute( "tr '[a-z]' '[A-Z]' <" .. path .. " >" .. out )
--     return( out )
--   end
--
--
_G['message_replace'] = function (path)

  --
  --  If the file doesn't reference GPG then we're OK to return
  -- early.
  --
  local found = false

  --
  -- We're going to invoke `mimegpg` and that will EITHER decrypt
  -- OR verify.
  --
  -- So we need to accept both kinds of messages here.
  --
  for line in io.lines(path) do
    if (line == "-----BEGIN PGP SIGNATURE-----") or (line == "-----BEGIN PGP MESSAGE-----") then
      found = true
    end
  end

  if found == false then
    return ""
  end
  --
  -- Do we have the binary we want?
  --
  if string.path "mimegpg" ~= "" then
    local out = os.tmpname()
    os.execute("mimegpg -d -c -- --batch < " .. path .. " > " .. out)
    return out
  else
    return ""
  end
end

local gpg = {}

--
-- Prompt for gpg options
--
gpg.prompt = function(options)
  gpg = Screen:prompt("(c)ancel, (n)othing, (s)ign, (e)ncryt, or (b)oth?", "nNcCsSeEbB")
  if (gpg == "c") or (gpg == "C") then
    return options
  elseif (gpg == "n") or (gpg == "N") then
    return ""
  elseif (gpg == "s") or (gpg == "S") then
    return "-s"
  elseif (gpg == "e") or (gpg == "E") then
    return "-E -- --batch -r ${recipient} --trust-model always"
  elseif (gpg == "b") or (gpg == "b") then
    return"-s -E -- --batch -r ${recipient} --trust-model always"
  end
end

--
-- Sign or encrypt a mail
--
gpg.prepare_mail = function(mail, options, recipient)
  local tmp2 = os.tmpname()
  -- Build up the command
  local cmd = "mimegpg " .. options .. "< " .. mail .. " > " .. tmp2
  -- Replace the recipient, if present.
  cmd = string.interp(cmd, { recipient = recipient:match("<(.*)>") or recipient, })
  -- Run the command.
  os.execute(cmd)
   -- Now replace the temporary file we're to use
  File:copy(tmp2, mail)
  os.remove(tmp2)
end

return gpg
