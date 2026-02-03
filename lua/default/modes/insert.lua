local Insert = {}

function Insert.init()
    Core.Keybinds.bind("insert", "<Esc>", function()
        Core.Modes.remove_minor_mode(Cini.workspace.viewport.doc, "insert")
    end)

    Core.Keybinds.bind("insert", "<Left>", function()
        Cini.workspace.viewport:move_cursor(Core.Cursor.left, 1)
    end)
    Core.Keybinds.bind("insert", "<Down>", function()
        Cini.workspace.viewport:move_cursor(Core.Cursor.down, 1)
    end)
    Core.Keybinds.bind("insert", "<Up>", function()
        Cini.workspace.viewport:move_cursor(Core.Cursor.up, 1)
    end)
    Core.Keybinds.bind("insert", "<Right>", function()
        Cini.workspace.viewport:move_cursor(Core.Cursor.right, 1)
    end)

    Core.Keybinds.bind("insert", "<Space>", function()
        local doc = Cini.workspace.viewport.doc

        doc:insert(doc.point, " ")
        Cini.workspace.viewport:move_cursor(Core.Cursor.right, 1)
    end)
    Core.Keybinds.bind("insert", "<Enter>", function()
        local doc = Cini.workspace.viewport.doc

        doc:insert(doc.point, "\n")
        Cini.workspace.viewport:move_cursor(Core.Cursor.down, 1)
        Cini.workspace.viewport:move_cursor(function(cur, d, _) cur:_jump_to_beginning_of_line(d) end, 1)
    end)
    Core.Keybinds.bind("insert", "<Tab>", function()
        local doc = Cini.workspace.viewport.doc

        doc:insert(doc.point, "\t")
        Cini.workspace.viewport:move_cursor(Core.Cursor.right, 1)
    end)
    Core.Keybinds.bind("insert", "<Bspc>", function()
        local doc = Cini.workspace.viewport.doc

        if doc.point ~= 0 and Cini.workspace.viewport:move_cursor(Core.Cursor.left, 1) then
            doc:remove(doc.point, doc.point + Core.Utf8.len(doc:slice(doc.point, doc.point + 1)))
        end
    end)
    Core.Keybinds.bind("insert", "<Del>", function()
        local doc = Cini.workspace.viewport.doc

        if doc.point ~= doc.size then
            doc:remove(doc.point, doc.point + Core.Utf8.len(doc:slice(doc.point, doc.point + 1)))
        end
    end)

    Core.Keybinds.bind("insert", "<CatchAll>", function(key_str)
        local doc = Cini.workspace.viewport.doc

        doc:insert(doc.point, key_str)
        Cini.workspace.viewport:move_cursor(Core.Cursor.right, Core.Utf8.count(key_str))

        return true
    end)
end

return Insert
