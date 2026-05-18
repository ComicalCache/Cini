--- @class Core.HoverActions
local HoverActions = {}

function HoverActions.init()
    Core.Hooks.add("cursor::after-move", {
            id = "hover_action.run",
            priority = 10,
            metadata = { description = "Executes a callback when hovering over text." }
        },
        function(view, pos)
            local action = view:get_view_property(pos, "hover_action")

            if action and type(action) == "function" then
                local ok, err = xpcall(action, debug.traceback, view)
                if not ok then
                    Cini:set_status_message("Failed to run action:\n" .. err, "error_message", 0, true)
                end
            end
        end)
end

return HoverActions
