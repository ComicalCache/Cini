local Insert = {}

function Insert.init()
    Core.Keybinds.bind("insert", "<Esc>", function()
        Core.Modes.remove_minor_mode(Cini.workspace.viewport.doc, "insert")
    end)

    Core.Keybinds.bind("insert", "<Left>", function()
        Cini.workspace.viewport:move_cursor(Core.Cursor.left, 1)
    end)
    Core.Keybinds.bind("insert", "<M-Left>", function()
        Cini.workspace.viewport:move_cursor(Core.Cursor._prev_word, 1)
    end)
    Core.Keybinds.bind("insert", "<Right>", function()
        Cini.workspace.viewport:move_cursor(Core.Cursor.right, 1)
    end)
    Core.Keybinds.bind("insert", "<M-Right>", function()
        Cini.workspace.viewport:move_cursor(Core.Cursor._next_word, 1)
    end)
    Core.Keybinds.bind("insert", "<Up>", function()
        Cini.workspace.viewport:move_cursor(Core.Cursor.up, 1)
    end)
    Core.Keybinds.bind("insert", "<Down>", function()
        Cini.workspace.viewport:move_cursor(Core.Cursor.down, 1)
    end)

    Core.Keybinds.bind("insert", "<Space>", function()
        local viewport = Cini.workspace.viewport
        local doc = viewport.doc

        doc:insert(viewport.cursor:point(doc), " ")
        Cini.workspace.viewport:move_cursor(Core.Cursor.right, 1)
    end)
    Core.Keybinds.bind("insert", "<Enter>", function()
        local viewport = Cini.workspace.viewport
        local doc = viewport.doc

        doc:insert(viewport.cursor:point(doc), "\n")
        Cini.workspace.viewport:move_cursor(Core.Cursor.down, 1)
        Cini.workspace.viewport:move_cursor(function(cur, d, _) cur:_jump_to_beginning_of_line(d) end, 1)
    end)
    Core.Keybinds.bind("insert", "<Tab>", function()
        local viewport = Cini.workspace.viewport
        local doc = viewport.doc

        doc:insert(viewport.cursor:point(doc), "\t")
        Cini.workspace.viewport:move_cursor(Core.Cursor.right, 1)
    end)
    Core.Keybinds.bind("insert", "<Bspc>", function()
        local viewport = Cini.workspace.viewport
        local doc = viewport.doc
        local point = viewport.cursor:point(doc)

        if point ~= 0 and Cini.workspace.viewport:move_cursor(Core.Cursor.left, 1) then
            doc:remove(point, point + Core.Utf8.len(doc:slice(point, point + 1)))
        end
    end)
    Core.Keybinds.bind("insert", "<Del>", function()
        local viewport = Cini.workspace.viewport
        local doc = viewport.doc
        local point = viewport.cursor:point(doc)

        if point ~= doc.size then
            doc:remove(point, point + Core.Utf8.len(doc:slice(point, point + 1)))
        end
    end)

    Core.Keybinds.bind("insert", "<CatchAll>", function(key_str)
        local viewport = Cini.workspace.viewport
        local doc = viewport.doc

        doc:insert(viewport.cursor:point(doc), key_str)
        Cini.workspace.viewport:move_cursor(Core.Cursor.right, Core.Utf8.count(key_str))

        return true
    end)
end

return Insert
