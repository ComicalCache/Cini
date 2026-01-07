local M = {}

function M.setup()
    local status_message = State.editor:get_mode("status_message")

    local err = Core.Face({ fg = Core.Rgb(181, 59, 59) })

    status_message:set_face("status_message:error", err)

    -- Color everything.
    status_message:set_syntax(".*", "status_message:error")
end

return M
