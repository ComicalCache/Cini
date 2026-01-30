local StatusMessage = {}

function StatusMessage.init()
    Core.Modes.register_mode("status_message", Core.Mode.new({
        name = "status_message",
        faces = {
            default = Core.Face({ fg = Core.Rgb(255, 100, 100), bg = Core.Rgb(41, 44, 51) }),
        },
    }))

    -- Override to not enter insert mode.
    Core.Keybinds.bind("status_message", "i", function() end)
end

return StatusMessage
