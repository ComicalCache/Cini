local M = {}

function M.setup()
    insert = State.editor:get_mode("insert")

    Keybind.bind("insert", "<Esc>", function(editor)
        editor.active_viewport.doc:remove_minor_mode("insert")
    end)

    Keybind.bind("insert", "<Left>", function(editor)
        editor.active_viewport:move_cursor(Core.Cursor.left, 1)
    end)
    Keybind.bind("insert", "<Down>", function(editor)
        editor.active_viewport:move_cursor(Core.Cursor.down, 1)
    end)
    Keybind.bind("insert", "<Up>", function(editor)
        editor.active_viewport:move_cursor(Core.Cursor.up, 1)
    end)
    Keybind.bind("insert", "<Right>", function(editor)
        editor.active_viewport:move_cursor(Core.Cursor.right, 1)
    end)

    Keybind.bind("insert", "<Space>", function(editor)
        local doc = editor.active_viewport.doc

        doc:insert(editor.active_viewport.cursor:byte(doc), " ")
        editor.active_viewport:move_cursor(Core.Cursor.right, 1)
    end)
    Keybind.bind("insert", "<Enter>", function(editor)
        local doc = editor.active_viewport.doc

        doc:insert(editor.active_viewport.cursor:byte(doc), "\n")
        editor.active_viewport:move_cursor(Core.Cursor.down, 1)
        editor.active_viewport:move_cursor(function(cur, doc, _) cur:jump_to_beginning_of_line(doc) end, 1)
    end)
    Keybind.bind("insert", "<Tab>", function(editor)
        local doc = editor.active_viewport.doc

        doc:insert(editor.active_viewport.cursor:byte(doc), "\t")
        editor.active_viewport:move_cursor(Core.Cursor.right, 1)
    end)
    Keybind.bind("insert", "<Bspc>", function(editor)
        local doc = editor.active_viewport.doc

        editor.active_viewport:move_cursor(Core.Cursor.left, 1)
        doc:remove(editor.active_viewport.cursor:byte(doc), 1)
    end)
    Keybind.bind("insert", "<Del>", function(editor)
        local doc = editor.active_viewport.doc

        local pos = editor.active_viewport.cursor:byte(doc)
        if pos ~= doc.size then
            doc:remove(pos, 1)
        end
    end)

    insert:bind_catch_all(function(editor, key)
        local doc = editor.active_viewport.doc
        local text = key:to_string()

        doc:insert(editor.active_viewport.cursor:byte(doc), text)
        editor.active_viewport:move_cursor(Core.Cursor.right, 1)

        return true
    end)
end

return M
