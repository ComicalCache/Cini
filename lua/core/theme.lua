local M = {}

function M.setup()
    global = State.editor:get_mode("global")

    default = Core.Face({ fg = Core.Rgb(200, 200, 200), bg = Core.Rgb(50, 50, 50) })
    gutter = Core.Face({ fg = Core.Rgb(150, 150, 150), bg = Core.Rgb(0, 0, 0) })
    whitespace = Core.Face({ fg = Core.Rgb(150, 150, 150) })
    err = Core.Face({ fg = Core.Rgb(150, 0, 0), bg = Core.Rgb(100, 0, 0) })

    global:set_face("default", default)
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
