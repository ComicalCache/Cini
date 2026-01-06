local M = {}

function M.setup()
    local global = State.editor:get_mode("global")

    local default = Core.Face({ fg = Core.Rgb(172, 178, 190), bg = Core.Rgb(41, 44, 51) })
    local mode_line_default = Core.Face({ fg = Core.Rgb(172, 178, 190), bg = Core.Rgb(59, 61, 66) })
    local gutter = Core.Face({ fg = Core.Rgb(101, 103, 105), bg = Core.Rgb(36, 40, 46) })
    local whitespace = Core.Face({ fg = Core.Rgb(68, 71, 79) })
    local err = Core.Face({ fg = Core.Rgb(181, 59, 59) })

    global:set_face("global:default", default)
    global:set_face("global:mode_line_default", mode_line_default)
    global:set_face("global:gutter", gutter)
    global:set_face("global:whitespace", whitespace)
    global:set_face("global:error", err)

    global:set_replacement(" ", "·", "global:whitespace")
    global:set_replacement("\t", "↦", "global:whitespace")
    global:set_replacement("\r", "↤", "global:whitespace")
    global:set_replacement("\n", "⏎", "global:whitespace")
    global:set_replacement("�", "�", "global:error")
end

return M
