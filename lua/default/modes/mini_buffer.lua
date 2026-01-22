local M = {}

function M.init()
    local Keybind = require("core.keybind")

    Keybind.bind("mini_buffer", "<Esc>", function(editor)
        editor:exit_mini_buffer()
    end)

    Keybind.bind("mini_buffer", "<Left>", function(editor)
        editor.mini_buffer:move_cursor(Core.Cursor.left, 1)
    end)
    Keybind.bind("mini_buffer", "<Right>", function(editor)
        editor.mini_buffer:move_cursor(Core.Cursor.right, 1)
    end)

    Keybind.bind("mini_buffer", "<Down>", function(editor) end)
    Keybind.bind("mini_buffer", "<Up>", function(editor) end)

    Keybind.bind("mini_buffer", "<Space>", function(editor)
        local doc = editor.mini_buffer.doc

        doc:insert(doc.point, " ")
        editor.mini_buffer:move_cursor(Core.Cursor.right, 1)
    end)
    Keybind.bind("mini_buffer", "<Enter>", function(editor)
        -- TODO: issue command
    end)
    Keybind.bind("mini_buffer", "<Tab>", function(editor) end)
    Keybind.bind("mini_buffer", "<Bspc>", function(editor)
        local doc = editor.mini_buffer.doc

        if doc.point ~= 0 then
            editor.mini_buffer:move_cursor(Core.Cursor.left, 1)
            doc:remove(doc.point, doc.point + 1)
        end
    end)
    Keybind.bind("mini_buffer", "<Del>", function(editor)
        local doc = editor.mini_buffer.doc

        if doc.point ~= doc.size then
            doc:remove(doc.point, doc.point + 1)
        end
    end)

    Keybind.bind("mini_buffer", "<CatchAll>", function(editor, key_str)
        local doc = editor.mini_buffer.doc

        doc:insert(doc.point, key_str)
        editor.mini_buffer:move_cursor(Core.Cursor.right, 1)

        return true
    end)
end

return M
