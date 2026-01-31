local MiniBuffer = {}

function MiniBuffer.init()
    Core.Modes.register_mode("error_message", Core.Mode.new({
        name = "error_message",
        faces = {
            default = Core.Face({ fg = Core.Rgb(224, 108, 117), bg = Core.Rgb(41, 44, 51) }),
        }
    }))
    Core.Modes.register_mode("info_message", Core.Mode.new({
        name = "info_message",
        faces = {
            default = Core.Face({ fg = Core.Rgb(97, 175, 239), bg = Core.Rgb(41, 44, 51) }),
        }
    }))

    Core.Hooks.add("mini_buffer::created", function()
        Core.Modes.set_major_mode(State.editor.workspace.mini_buffer.doc, "mini_buffer")
    end)

    Core.Keybinds.bind("mini_buffer", "<Esc>", function()
        State.editor.workspace:exit_mini_buffer()
    end)

    Core.Keybinds.bind("mini_buffer", "<Left>", function()
        State.editor.workspace.mini_buffer:move_cursor(Core.Cursor.left, 1)
    end)
    Core.Keybinds.bind("mini_buffer", "<Right>", function()
        State.editor.workspace.mini_buffer:move_cursor(Core.Cursor.right, 1)
    end)

    Core.Keybinds.bind("mini_buffer", "<Down>", function()
        State.editor.workspace.mini_buffer:move_cursor(Core.Cursor.down, 1)
    end)
    Core.Keybinds.bind("mini_buffer", "<Up>", function()
        State.editor.workspace.mini_buffer:move_cursor(Core.Cursor.up, 1)
    end)

    Core.Keybinds.bind("mini_buffer", "<Space>", function()
        local doc = State.editor.workspace.mini_buffer.doc

        doc:insert(doc.point, " ")
        State.editor.workspace.mini_buffer:move_cursor(Core.Cursor.right, 1)
    end)
    Core.Keybinds.bind("mini_buffer", "<S-Enter>", function()
        local doc = State.editor.workspace.mini_buffer.doc

        doc:insert(doc.point, "\n")
        State.editor.workspace.mini_buffer:move_cursor(Core.Cursor.down, 1)
        State.editor.workspace.mini_buffer:move_cursor(function(cur, d, _) cur:_jump_to_beginning_of_line(d) end, 1)
    end)
    Core.Keybinds.bind("mini_buffer", "<Tab>", function() end)
    Core.Keybinds.bind("mini_buffer", "<Bspc>", function()
        local doc = State.editor.workspace.mini_buffer.doc

        if doc.point ~= 0 then
            State.editor.workspace.mini_buffer:move_cursor(Core.Cursor.left, 1)
            doc:remove(doc.point, doc.point + Core.Utf8.len(doc:slice(doc.point, doc.point + 1)))
        end
    end)
    Core.Keybinds.bind("mini_buffer", "<Del>", function()
        local doc = State.editor.workspace.mini_buffer.doc

        if doc.point ~= doc.size then
            doc:remove(doc.point, doc.point + Core.Utf8.len(doc:slice(doc.point, doc.point + 1)))
        end
    end)

    Core.Keybinds.bind("mini_buffer", "<CatchAll>", function(key_str)
        local doc = State.editor.workspace.mini_buffer.doc

        doc:insert(doc.point, key_str)
        State.editor.workspace.mini_buffer:move_cursor(Core.Cursor.right, Core.Utf8.count(key_str))

        return true
    end)
end

return MiniBuffer
