local M = {}

function M.init()
    local Keybind = require("core.keybind")

    Keybind.bind("insert", "<Esc>", function(editor)
        local Mode = require("core.mode")

        Mode.remove_minor_mode(editor.viewport.doc, "insert")
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

        doc:insert(doc.point, " ")
        editor.viewport:move_cursor(Core.Cursor.right, 1)
    end)
    Keybind.bind("insert", "<Enter>", function(editor)
        local doc = editor.viewport.doc

        doc:insert(doc.point, "\n")
        editor.viewport:move_cursor(Core.Cursor.down, 1)
        editor.viewport:move_cursor(function(cur, d, _) cur:_jump_to_beginning_of_line(d) end, 1)
    end)
    Keybind.bind("insert", "<Tab>", function(editor)
        local doc = editor.viewport.doc

        doc:insert(doc.point, "\t")
        editor.viewport:move_cursor(Core.Cursor.right, 1)
    end)
    Keybind.bind("insert", "<Bspc>", function(editor)
        local doc = editor.viewport.doc

        if doc.point ~= 0 then
            editor.viewport:move_cursor(Core.Cursor.left, 1)
            doc:remove(doc.point, doc.point + 1)
        end
    end)
    Keybind.bind("insert", "<Del>", function(editor)
        local doc = editor.viewport.doc

        if doc.point ~= doc.size then
            doc:remove(doc.point, doc.point + 1)
        end
    end)

    Keybind.bind("insert", "<CatchAll>", function(editor, key_str)
        local doc = editor.viewport.doc

        doc:insert(doc.point, key_str)
        editor.viewport:move_cursor(Core.Cursor.right, 1)

        return true
    end)
end

return M
