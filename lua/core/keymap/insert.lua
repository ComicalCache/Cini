local M = {}

function M.setup()
    local insert = State.editor:get_mode("insert")

    Keybind.bind("insert", "<Esc>", function(editor)
        editor.viewport.doc:remove_minor_mode("insert")
    end)

    Keybind.bind("insert", "<Left>", function(editor)
        editor.viewport:move_cursor(Core.Cursor.left, 1)
    end)
    Keybind.bind("insert", "<Down>", function(editor)
        editor.viewport:move_cursor(Core.Cursor.down, 1)
    end)
    Keybind.bind("insert", "<Up>", function(editor)
        editor.viewport:move_cursor(Core.Cursor.up, 1)
    end)
    Keybind.bind("insert", "<Right>", function(editor)
        editor.viewport:move_cursor(Core.Cursor.right, 1)
    end)

    Keybind.bind("insert", "<Space>", function(editor)
        local doc = editor.viewport.doc

        doc:insert(editor.viewport.cursor:point(doc), " ")
        editor.viewport:move_cursor(Core.Cursor.right, 1)
    end)
    Keybind.bind("insert", "<Enter>", function(editor)
        local doc = editor.viewport.doc

        doc:insert(editor.viewport.cursor:point(doc), "\n")
        editor.viewport:move_cursor(Core.Cursor.down, 1)
        editor.viewport:move_cursor(function(cur, doc, _) cur:jump_to_beginning_of_line(doc) end, 1)
    end)
    Keybind.bind("insert", "<Tab>", function(editor)
        local doc = editor.viewport.doc

        doc:insert(editor.viewport.cursor:point(doc), "\t")
        editor.viewport:move_cursor(Core.Cursor.right, 1)
    end)
    Keybind.bind("insert", "<Bspc>", function(editor)
        local doc = editor.viewport.doc

        editor.viewport:move_cursor(Core.Cursor.left, 1)
        doc:remove(editor.viewport.cursor:point(doc), 1)
    end)
    Keybind.bind("insert", "<Del>", function(editor)
        local doc = editor.viewport.doc

        local pos = editor.viewport.cursor:point(doc)
        if pos ~= doc.size then
            doc:remove(pos, 1)
        end
    end)

    insert:bind_catch_all(function(editor, key)
        local doc = editor.viewport.doc
        local text = key:to_string()

        doc:insert(editor.viewport.cursor:point(doc), text)
        editor.viewport:move_cursor(Core.Cursor.right, 1)

        return true
    end)
end

return M
