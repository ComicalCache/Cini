local M = {}

function M.setup()
    global = State.editor:get_mode("global")

    default = Core.Face({ fg = Core.Rgb(172, 178, 190), bg = Core.Rgb(41, 44, 51) })
    mode_line_default = Core.Face({ fg = Core.Rgb(172, 178, 190), bg = Core.Rgb(59, 61, 66) })
    gutter = Core.Face({ fg = Core.Rgb(101, 103, 105), bg = Core.Rgb(36, 40, 46) })
    whitespace = Core.Face({ fg = Core.Rgb(68, 71, 79) })
    err = Core.Face({ fg = Core.Rgb(181, 59, 59) })

    global:set_face("default", default)
    global:set_face("mode_line_default", mode_line_default)
    global:set_face("gutter", gutter)
    global:set_face("whitespace", whitespace)
    global:set_face("error", err)

    global:set_replacement(" ", "·", "whitespace")
    global:set_replacement("\t", "↦", "whitespace")
    global:set_replacement("\r", "↤", "whitespace")
    global:set_replacement("\n", "⏎", "whitespace")
    global:set_replacement("�", "�", "error")
end

return M
