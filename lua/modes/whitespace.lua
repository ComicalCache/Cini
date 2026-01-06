local M = {}

function M.setup()
    whitespace = State.editor:get_mode("whitespace")

    warn = Core.Face({ fg = Core.Rgb(68, 71, 79), bg = Core.Rgb(181, 59, 59) })

    global:set_face("warn", warn)

    whitespace:set_replacement("\t", "↦", "warn")
    whitespace:set_syntax("[ \\t]+(?=[\\r\\n]*$)", "warn")
end

return M
