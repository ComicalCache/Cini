local MiniBuffer = {}

function MiniBuffer.setup()
    -- Commands.
    Core.Commands.register("mini_buffer.mac_backslash", {
        metadata = {
            modifies = true,
            synopsis = "Insert backslash",
            description = "Inserts a backslash character (macOS German layout workaround)."
        },
        callback = function()
            local view = Cini.workspace.mini_buffer.view
            view.doc:insert(view.cur:point(view), "\\")
            view:move_cursor(Core.Cursor.right, 1)

            return true
        end
    })

    -- Keybinds.
    Core.Keybinds.bind("mini_buffer", "<M-S-7>", "mini_buffer.mac_backslash")
end

function MiniBuffer.init() end

return MiniBuffer
