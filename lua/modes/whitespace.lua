local M = {}

function M.setup()
    whitespace = State.editor:get_mode("whitespace")

    warn = Core.Face({ fg = Core.Rgb(150, 150, 0), bg = Core.Rgb(200, 200, 0) })

    global:set_face("warn", warn)

    whitespace:set_replacement("\t", "↦", "warn")
    whitespace:set_syntax("[ \\t]+(?=[\\r\\n]*$)", "warn")
end

return M
