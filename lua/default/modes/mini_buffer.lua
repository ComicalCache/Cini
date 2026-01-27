local MiniBuffer = {}

function MiniBuffer.init()
    Core.Hooks.add("mini_buffer::created", function(mini_buffer)
        Core.Modes.set_major_mode(mini_buffer.doc, "mini_buffer")
    end)

    Core.Keybinds.bind("mini_buffer", "<Esc>", function(editor)
        editor:exit_mini_buffer()
    end)

    Core.Keybinds.bind("mini_buffer", "<Left>", function(editor)
        editor.mini_buffer:move_cursor(Core.Cursor.left, 1)
    end)
    Core.Keybinds.bind("mini_buffer", "<Right>", function(editor)
        editor.mini_buffer:move_cursor(Core.Cursor.right, 1)
    end)

    Core.Keybinds.bind("mini_buffer", "<Down>", function(editor)
        editor.viewport:move_cursor(Core.Cursor.down, 1)
    end)
    Core.Keybinds.bind("mini_buffer", "<Up>", function(editor)
        editor.viewport:move_cursor(Core.Cursor.up, 1)
    end)

    Core.Keybinds.bind("mini_buffer", "<Space>", function(editor)
        local doc = editor.mini_buffer.doc

        doc:insert(doc.point, " ")
        editor.mini_buffer:move_cursor(Core.Cursor.right, 1)
    end)
    Core.Keybinds.bind("mini_buffer", "<S-Enter>", function(editor)
        local doc = editor.mini_buffer.doc

        doc:insert(doc.point, "\n")
        editor.mini_buffer:move_cursor(Core.Cursor.down, 1)
        editor.mini_buffer:move_cursor(function(cur, d, _) cur:_jump_to_beginning_of_line(d) end, 1)
    end)
    Core.Keybinds.bind("mini_buffer", "<Tab>", function(_) end)
    Core.Keybinds.bind("mini_buffer", "<Bspc>", function(editor)
        local doc = editor.mini_buffer.doc

        if doc.point ~= 0 then
            editor.mini_buffer:move_cursor(Core.Cursor.left, 1)
            doc:remove(doc.point, doc.point + 1)
        end
    end)
    Core.Keybinds.bind("mini_buffer", "<Del>", function(editor)
        local doc = editor.mini_buffer.doc

        if doc.point ~= doc.size then
            doc:remove(doc.point, doc.point + 1)
        end
    end)

    Core.Keybinds.bind("mini_buffer", "<Enter>", function(_)
        -- TODO: issue command.
    end)

    Core.Keybinds.bind("mini_buffer", "<CatchAll>", function(editor, key_str)
        local doc = editor.mini_buffer.doc

        doc:insert(doc.point, key_str)
        editor.mini_buffer:move_cursor(Core.Cursor.right, 1)

        return true
    end)
end

return MiniBuffer
