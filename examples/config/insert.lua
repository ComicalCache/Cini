local Insert = {}

function Insert.setup()
    -- Commands.
    Core.Commands.register("insert.mac_backslash", {
        metadata = {
            modifies = true,
            synopsis = "Insert backslash",
            description = "Inserts a backslash character (macOS German layout workaround)."
        },
        callback = function()
            local view = Cini.workspace.viewport.view
            view.doc:insert(view.cur:point(view), "\\")
            view:move_cursor(Core.Cursor.right, 1)

            return true
        end
    })

    -- Keybinds.
    Core.Keybinds.bind("insert", "<M-S-7>", "insert.mac_backslash")
end

function Insert.init() end

return Insert
