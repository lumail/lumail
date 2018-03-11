--
-- This file demonstrates how the `index_view()` function could be overridden to
-- make threads in the index_view collapseable (based on Message-Id and
-- In-Reply-To headers).
--
-- For collapsing threads, "index.expand" must be set to zero.
-- If it is not set, the function behaves like the original `index_view()` and
-- assumes "index.expand" to be 1 (one).
--

-- helper for checking whether there is a key in an array
function setContains(set, key)
    for i, e in ipairs(set) do
        if key == e then
            return true
        end
    end
    return false
end

function index_view()
    local result = {}

    -- Get the available messages.
    local messages = get_messages()

    if (messages == nil) or (#messages == 0) then
        Screen:draw(10, 10, "There are no visible messages.")
        return result
    end

    -- Get the current offset
    local mode = Config:get "global.mode"
    local cur = tonumber(Config.get_with_default("index.current", 0))

    --
    -- If the current line is bigger than the count of messages
    -- then something has gone wrong.
    --
    if cur >= #messages then
        cur = #messages - 1
    end

    -- Find the height of the screen.
    local height = Screen:height()

    -- The minimum message number we're going to format
    local min = cur - height
    if min < 0 then
        min = 0
    end

    -- The maximum message-number we're going to format.
    local max = cur + height

    -- Are we optimizing?
    local fast = Config.get_with_default("index.fast", 0)

    local expand       = Config.get_with_default("index.expand", 1)
    local previous_ids = {}

    for offset, object in ipairs(messages) do
        local str = "INVISIBLE"

        -- If optimizing
        if fast ~= 0 then

            --
            -- Only show the message if it will fit on the screen
            --
            if (offset >= min) and (offset < max) then
                str = object:format(threads_indentation[object], offset)
            else
                str = "INVISIBLE - Outside the viewport!"
            end
        else
            -- Else format all entries
            str = object:format(threads_indentation[object], offset)
        end

        if expand == 1 then
            table.insert(result, str)
        else
            local replyto = object:header("In-Reply-To")
            if replyto == "" or replyto == nil then
                table.insert(result, str)
            else
                if not setContains(previous_ids, replyto) then
                    table.insert(result, str)
                end
            end
        end

        table.insert(previous_ids, object:header("Message-Id"))
    end

    --
    -- Update the colours
    --
    result = add_colours(result, 'index')
    return result
end

