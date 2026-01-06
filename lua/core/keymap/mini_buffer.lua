local M = {}

function M.setup()
    mini_buffer = State.editor:get_mode("mini_buffer")

    Keybind.bind("mini_buffer", "<Esc>", function(editor)
        editor:exit_mini_buffer()
    end)

    Keybind.bind("mini_buffer", "<Left>", function(editor)
        editor.active_viewport:move_cursor(Core.Cursor.left, 1)
    end)
    Keybind.bind("mini_buffer", "<Right>", function(editor)
        editor.active_viewport:move_cursor(Core.Cursor.right, 1)
    end)

    Keybind.bind("mini_buffer", "<Down>", function(editor) end)
    Keybind.bind("mini_buffer", "<Up>", function(editor) end)

    Keybind.bind("mini_buffer", "<Space>", function(editor)
        local doc = editor.active_viewport.doc

        doc:insert(editor.active_viewport.cursor:byte(doc), " ")
        editor.active_viewport:move_cursor(Core.Cursor.right, 1)
    end)
    Keybind.bind("mini_buffer", "<Enter>", function(editor)
        -- TODO: issue command
    end)
    Keybind.bind("mini_buffer", "<Tab>", function(editor) end)
    Keybind.bind("mini_buffer", "<Bspc>", function(editor)
        local doc = editor.active_viewport.doc

        editor.active_viewport:move_cursor(Core.Cursor.left, 1)
        doc:remove(editor.active_viewport.cursor:byte(doc), 1)
    end)
    Keybind.bind("mini_buffer", "<Del>", function(editor)
        local doc = editor.active_viewport.doc

        local pos = editor.active_viewport.cursor:byte(doc)
        if pos ~= doc.size then
            doc:remove(pos, 1)
        end
    end)

    mini_buffer:bind_catch_all(function(editor, key)
        local doc = editor.active_viewport.doc
        local text = key:to_string()
        doc:insert(editor.active_viewport.cursor:byte(doc), text)
        editor.active_viewport:move_cursor(Core.Cursor.right, 1)

        return true
    end)
end

return M
