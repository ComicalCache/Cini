local Insert = {}

function Insert.init()
    Core.Keybinds.bind("insert", "<Esc>", function()
        Core.Modes.remove_minor_mode(State.editor.workspace.viewport.doc, "insert")
    end)

    Core.Keybinds.bind("insert", "<Left>", function()
        State.editor.workspace.viewport:move_cursor(Core.Cursor.left, 1)
    end)
    Core.Keybinds.bind("insert", "<Down>", function()
        State.editor.workspace.viewport:move_cursor(Core.Cursor.down, 1)
    end)
    Core.Keybinds.bind("insert", "<Up>", function()
        State.editor.workspace.viewport:move_cursor(Core.Cursor.up, 1)
    end)
    Core.Keybinds.bind("insert", "<Right>", function()
        State.editor.workspace.viewport:move_cursor(Core.Cursor.right, 1)
    end)

    Core.Keybinds.bind("insert", "<Space>", function()
        local doc = State.editor.workspace.viewport.doc

        doc:insert(doc.point, " ")
        State.editor.workspace.viewport:move_cursor(Core.Cursor.right, 1)
    end)
    Core.Keybinds.bind("insert", "<Enter>", function()
        local doc = State.editor.workspace.viewport.doc

        doc:insert(doc.point, "\n")
        State.editor.workspace.viewport:move_cursor(Core.Cursor.down, 1)
        State.editor.workspace.viewport:move_cursor(function(cur, d, _) cur:_jump_to_beginning_of_line(d) end, 1)
    end)
    Core.Keybinds.bind("insert", "<Tab>", function()
        local doc = State.editor.workspace.viewport.doc

        doc:insert(doc.point, "\t")
        State.editor.workspace.viewport:move_cursor(Core.Cursor.right, 1)
    end)
    Core.Keybinds.bind("insert", "<Bspc>", function()
        local doc = State.editor.workspace.viewport.doc

        if doc.point ~= 0 then
            State.editor.workspace.viewport:move_cursor(Core.Cursor.left, 1)
            doc:remove(doc.point, doc.point + Core.Utf8.len(doc:slice(doc.point, doc.point + 1)))
        end
    end)
    Core.Keybinds.bind("insert", "<Del>", function()
        local doc = State.editor.workspace.viewport.doc

        if doc.point ~= doc.size then
            doc:remove(doc.point, doc.point + Core.Utf8.len(doc:slice(doc.point, doc.point + 1)))
        end
    end)

    Core.Keybinds.bind("insert", "<CatchAll>", function(key_str)
        local doc = State.editor.workspace.viewport.doc

        doc:insert(doc.point, key_str)
        State.editor.workspace.viewport:move_cursor(Core.Cursor.right, Core.Utf8.count(key_str))

        return true
    end)
end

return Insert
