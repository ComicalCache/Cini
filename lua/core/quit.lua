--- @class Core.Quit
local Quit = {}

function Quit.init()
    Core.Quit = Quit
end

--- Asks to discard unsaved changes before stopping the event loop and quitting Cini.
function Quit.safe_quit()
    local count = 0
    local name = ""

    -- Check all documents for the modified flag
    for _, doc in ipairs(Cini.documents) do
        if doc.modified then
            count = count + 1
            name = doc.path or "Scratchpad"
        end
    end

    if count == 0 then
        Cini:quit()
    else
        local msg = ""
        if count == 1 then
            msg = string.format("%s has unsaved changes. Discard unsaved changes? (y/n) ", name)
        else
            msg = string.format("%d documents have unsaved changes. Discard unsaved changes? (y/n) ", count)
        end

        Core.Prompt.run(msg, nil, function(sel) if sel:lower() == "y" then Cini:quit() end end)
    end
end

return Quit
