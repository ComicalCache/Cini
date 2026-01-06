local M = {}

function M.setup()
    local whitespace = State.editor:get_mode("whitespace")

    local warn = Core.Face({ fg = Core.Rgb(68, 71, 79), bg = Core.Rgb(181, 59, 59) })

    whitespace:set_face("whitespace:warn", warn)

    whitespace:set_replacement("\t", "↦", "whitespace:warn")
    whitespace:set_syntax("[ \\t]+(?=[\\r\\n]*$)", "whitespace:warn")
end

return M
