local M = {}

function M.setup()
    local mini_buffer = State.editor:get_mode("mini_buffer")

    Keybind.bind("mini_buffer", "<Esc>", function(editor)
        editor:exit_mini_buffer()
    end)

    Keybind.bind("mini_buffer", "<Left>", function(editor)
        editor.viewport:move_cursor(Core.Cursor.left, 1)
    end)
    Keybind.bind("mini_buffer", "<Right>", function(editor)
        editor.viewport:move_cursor(Core.Cursor.right, 1)
    end)

    Keybind.bind("mini_buffer", "<Down>", function(editor) end)
    Keybind.bind("mini_buffer", "<Up>", function(editor) end)

    Keybind.bind("mini_buffer", "<Space>", function(editor)
        local doc = editor.viewport.doc

        doc:insert(editor.viewport.cursor:point(doc), " ")
        editor.viewport:move_cursor(Core.Cursor.right, 1)
    end)
    Keybind.bind("mini_buffer", "<Enter>", function(editor)
        -- TODO: issue command
    end)
    Keybind.bind("mini_buffer", "<Tab>", function(editor) end)
    Keybind.bind("mini_buffer", "<Bspc>", function(editor)
        local doc = editor.viewport.doc

        editor.viewport:move_cursor(Core.Cursor.left, 1)
        doc:remove(editor.viewport.cursor:point(doc), 1)
    end)
    Keybind.bind("mini_buffer", "<Del>", function(editor)
        local doc = editor.viewport.doc

        local pos = editor.viewport.cursor:point(doc)
        if pos ~= doc.size then
            doc:remove(pos, 1)
        end
    end)

    mini_buffer:bind_catch_all(function(editor, key)
        local doc = editor.viewport.doc
        local text = key:to_string()
        doc:insert(editor.viewport.cursor:point(doc), text)
        editor.viewport:move_cursor(Core.Cursor.right, 1)

        return true
    end)
end

return M
