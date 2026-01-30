local MiniBuffer = {}

function MiniBuffer.init()
    Core.Hooks.add("mini_buffer::created", function(mini_buffer)
        Core.Modes.set_major_mode(mini_buffer.doc, "mini_buffer")
    end)

    Core.Keybinds.bind("mini_buffer", "<Esc>", function()
        State.editor:exit_mini_buffer()
    end)

    Core.Keybinds.bind("mini_buffer", "<Left>", function()
        State.editor.mini_buffer:move_cursor(Core.Cursor.left, 1)
    end)
    Core.Keybinds.bind("mini_buffer", "<Right>", function()
        State.editor.mini_buffer:move_cursor(Core.Cursor.right, 1)
    end)

    Core.Keybinds.bind("mini_buffer", "<Down>", function()
        State.editor.mini_buffer:move_cursor(Core.Cursor.down, 1)
    end)
    Core.Keybinds.bind("mini_buffer", "<Up>", function()
        State.editor.mini_buffer:move_cursor(Core.Cursor.up, 1)
    end)

    Core.Keybinds.bind("mini_buffer", "<Space>", function()
        local doc = State.editor.mini_buffer.doc

        doc:insert(doc.point, " ")
        State.editor.mini_buffer:move_cursor(Core.Cursor.right, 1)
    end)
    Core.Keybinds.bind("mini_buffer", "<S-Enter>", function()
        local doc = State.editor.mini_buffer.doc

        doc:insert(doc.point, "\n")
        State.editor.mini_buffer:move_cursor(Core.Cursor.down, 1)
        State.editor.mini_buffer:move_cursor(function(cur, d, _) cur:_jump_to_beginning_of_line(d) end, 1)
    end)
    Core.Keybinds.bind("mini_buffer", "<Tab>", function() end)
    Core.Keybinds.bind("mini_buffer", "<Bspc>", function()
        local doc = State.editor.mini_buffer.doc

        if doc.point ~= 0 then
            State.editor.mini_buffer:move_cursor(Core.Cursor.left, 1)
            doc:remove(doc.point, doc.point + 1)
        end
    end)
    Core.Keybinds.bind("mini_buffer", "<Del>", function()
        local doc = State.editor.mini_buffer.doc

        if doc.point ~= doc.size then
            doc:remove(doc.point, doc.point + 1)
        end
    end)

    Core.Keybinds.bind("mini_buffer", "<Enter>", function()
        -- TODO: issue command.
    end)

    Core.Keybinds.bind("mini_buffer", "<CatchAll>", function(key_str)
        local doc = State.editor.mini_buffer.doc

        doc:insert(doc.point, key_str)
        State.editor.mini_buffer:move_cursor(Core.Cursor.right, 1)

        return true
    end)
end

return MiniBuffer
