
folders = get_maildirs( os.getenv("HOME") .. "/Maildir/" )

for k,v in ipairs( folders ) do
   print( k , v:path() )
   print( "\tThere are " .. v:total_messages() .. " total messages in " .. v:path() )
   print( "\tThere are " .. v:unread_messages() .. " unread messages in " .. v:path() )

end


math.randomseed(os.time())

--
--  Pick a random folder.
--
random_folder = folders[math.random(#folders)]
print( "Random Folder : " .. random_folder:path() )

--
--  Pick a random message
--
messages = random_folder:messages()
random_message = messages[math.random(#messages)]

--
-- Now show  some simple headers headers
--
local headers = random_message:headers()
if ( headers['subject'] ) then
   print( "\tSubject: " .. headers['subject'] )
end
if ( headers['from'] ) then
   print( "\tSender: " .. random_message:headers()['from'] )
end

--
-- Output the headers it has.
--
for k,v in pairs( random_message:headers() ) do
   print( "Header " .. k .. " has value '" .. v .. "'"  )
end


os.exit(0)
