local HoverActions = {}

function HoverActions.init()
    Core.Hooks.add("cursor::after-move", function(doc)
        local action = doc:get_text_property(doc.point, "hover-action")

        if action and type(action) == "function" then
            local ok, err = xpcall(action, debug.traceback, doc)
            if not ok then
                Cini:set_status_message("Failed to run action:\n" .. err, "error_message", 0, true)
            end
        end
    end)
end

return HoverActions
