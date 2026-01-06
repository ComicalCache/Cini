local M = {}

function M.setup()
    insert = State.editor:get_mode("insert")

    Keybind.bind("insert", "<Esc>", function(editor)
        editor.active_viewport.doc:remove_minor_mode("insert")
    end)

    Keybind.bind("insert", "<Left>", function(editor)
        editor.active_viewport:cursor_left(1)
    end)
    Keybind.bind("insert", "<Down>", function(editor)
        editor.active_viewport:cursor_down(1)
    end)
    Keybind.bind("insert", "<Up>", function(editor)
        editor.active_viewport:cursor_up(1)
    end)
    Keybind.bind("insert", "<Right>", function(editor)
        editor.active_viewport:cursor_right(1)
    end)

    insert:bind_catch_all(function(editor, key)
        local doc = editor.active_viewport.doc
        local text = key:to_string()
        doc:insert(editor.active_viewport.cursor:byte(doc), text)
        editor.active_viewport:move_cursor(Core.Cursor.right, #text)

        return true
    end)
end

return M
