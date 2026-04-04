local Insert = {}

function Insert.init()
    Core.Modes.register_mode({
        name = "insert",
        cursor_style = Core.CursorStyle.BlinkingBar
    })

    -- Enter insert mode.
    Core.Commands.register("global.insert_mode", {
        metadata = { modifies = true },
        run = function()
            local viewport = Cini.workspace.viewport
            viewport.doc:begin_transaction(viewport.cursor:point(viewport.doc))

            Core.Modes.add_minor_mode(viewport.doc, "insert")
        end
    })
    Core.Commands.register("global.insert_mode_after", {
        metadata = { modifies = true },
        run = function()
            local viewport = Cini.workspace.viewport
            viewport.doc:begin_transaction(viewport.cursor:point(viewport.doc))

            viewport:move_cursor(Core.Cursor.right, 1)
            Core.Modes.add_minor_mode(viewport.doc, "insert")
        end
    })
    Core.Commands.register("global.insert_mode_end_of_line", {
        metadata = { modifies = true },
        run = function()
            local viewport = Cini.workspace.viewport
            viewport.doc:begin_transaction(viewport.cursor:point(viewport.doc))

            Cini.workspace.viewport:move_cursor(function(cur, doc, _) cur:_jump_to_end_of_line(doc) end, 1)
            Core.Modes.add_minor_mode(Cini.workspace.viewport.doc, "insert")
        end
    })
    Core.Commands.register("global.insert_newline_below", {
        metadata = { modifies = true },
        run = function()
            local viewport = Cini.workspace.viewport
            local doc = viewport.doc

            doc:begin_transaction(viewport.cursor:point(doc))

            viewport:move_cursor(function(cur, d, _) cur:_jump_to_end_of_line(d) end, 1)
            doc:insert(viewport.cursor:point(doc), "\n")
            viewport:move_cursor(Core.Cursor.right, 1)

            Core.Modes.add_minor_mode(doc, "insert")
        end
    })
    Core.Commands.register("global.insert_newline_above", {
        metadata = { modifies = true },
        run = function()
            local viewport = Cini.workspace.viewport
            local doc = viewport.doc

            doc:begin_transaction(viewport.cursor:point(viewport.doc))

            viewport:move_cursor(function(cur, d, _) cur:_jump_to_beginning_of_line(d) end, 1)
            doc:insert(viewport.cursor:point(doc), "\n")
            viewport:move_cursor(function(cur, d, _) cur:_jump_to_beginning_of_line(d) end, 1)

            Core.Modes.add_minor_mode(doc, "insert")
        end
    })
    Core.Keybinds.bind("global", "i", "global.insert_mode")
    Core.Keybinds.bind("global", "a", "global.insert_mode_after")
    Core.Keybinds.bind("global", "A", "global.insert_mode_end_of_line")
    Core.Keybinds.bind("global", "o", "global.insert_newline_below")
    Core.Keybinds.bind("global", "O", "global.insert_newline_above")

    -- Exit.
    Core.Commands.register("insert.exit", {
        metadata = { modifies = false },
        run = function()
            local viewport = Cini.workspace.viewport
            viewport.doc:end_transaction(viewport.cursor:point(viewport.doc))

            Core.Modes.remove_minor_mode(Cini.workspace.viewport.doc, "insert")
        end
    })
    Core.Keybinds.bind("insert", "<Esc>", "insert.exit")

    -- Movement.
    Core.Commands.register("insert.move_left", {
        metadata = { modifies = false },
        run = function() Cini.workspace.viewport:move_cursor(Core.Cursor.left, 1) end
    })
    Core.Commands.register("insert.move_prev_word", {
        metadata = { modifies = false },
        run = function() Cini.workspace.viewport:move_cursor(Core.Cursor._prev_word, 1) end
    })
    Core.Commands.register("insert.move_right", {
        metadata = { modifies = false },
        run = function() Cini.workspace.viewport:move_cursor(Core.Cursor.right, 1) end
    })
    Core.Commands.register("insert.move_next_word", {
        metadata = { modifies = false },
        run = function() Cini.workspace.viewport:move_cursor(Core.Cursor._next_word, 1) end
    })
    Core.Commands.register("insert.move_up", {
        metadata = { modifies = false },
        run = function() Cini.workspace.viewport:move_cursor(Core.Cursor.up, 1) end
    })
    Core.Commands.register("insert.move_down", {
        metadata = { modifies = false },
        run = function() Cini.workspace.viewport:move_cursor(Core.Cursor.down, 1) end
    })
    Core.Keybinds.bind("insert", "<Left>", "insert.move_left")
    Core.Keybinds.bind("insert", "<M-Left>", "insert.move_prev_word")
    Core.Keybinds.bind("insert", "<Right>", "insert.move_right")
    Core.Keybinds.bind("insert", "<M-Right>", "insert.move_next_word")
    Core.Keybinds.bind("insert", "<Up>", "insert.move_up")
    Core.Keybinds.bind("insert", "<Down>", "insert.move_down")

    -- Modify text.
    Core.Commands.register("insert.space", {
        metadata = { modifies = true },
        run = function()
            local viewport = Cini.workspace.viewport
            local doc = viewport.doc

            doc:insert(viewport.cursor:point(viewport.doc), " ")
            viewport:move_cursor(Core.Cursor.right, 1)

            -- End transaction on whitespace.
            local pos = viewport.cursor:point(doc)
            doc:end_transaction(pos)
            doc:begin_transaction(pos)
        end
    })
    Core.Commands.register("insert.enter", {
        metadata = { modifies = true },
        run = function()
            local viewport = Cini.workspace.viewport
            local doc = viewport.doc

            doc:insert(viewport.cursor:point(doc), "\n")
            viewport:move_cursor(Core.Cursor.down, 1)
            viewport:move_cursor(function(cur, d, _) cur:_jump_to_beginning_of_line(d) end, 1)

            -- End transaction on whitespace.
            local pos = viewport.cursor:point(doc)
            doc:end_transaction(pos)
            doc:begin_transaction(pos)
        end
    })
    Core.Commands.register("insert.tab", {
        metadata = { modifies = true },
        run = function()
            local viewport = Cini.workspace.viewport
            local doc = viewport.doc

            doc:insert(viewport.cursor:point(doc), "\t")
            viewport:move_cursor(Core.Cursor.right, 1)

            -- End transaction on whitespace.
            local pos = viewport.cursor:point(doc)
            doc:end_transaction(pos)
            doc:begin_transaction(pos)
        end
    })
    Core.Commands.register("insert.backspace", {
        metadata = { modifies = true },
        run = function()
            local viewport = Cini.workspace.viewport
            local doc = viewport.doc
            local point = viewport.cursor:point(doc)

            if point ~= 0 and viewport:move_cursor(Core.Cursor.left, 1) then
                doc:remove(viewport.cursor:point(doc), point)
            end
        end
    })
    Core.Commands.register("insert.delete", {
        metadata = { modifies = true },
        run = function()
            local viewport = Cini.workspace.viewport
            local doc = viewport.doc
            local point = viewport.cursor:point(doc)

            if point ~= doc.size then doc:remove(point, point + Core.Utf8.len(doc:slice(point, point + 1))) end
        end
    })
    Core.Keybinds.bind("insert", "<Space>", "insert.space")
    Core.Keybinds.bind("insert", "<Enter>", "insert.enter")
    Core.Keybinds.bind("insert", "<Tab>", "insert.tab")
    Core.Keybinds.bind("insert", "<Bspc>", "insert.backspace")
    Core.Keybinds.bind("insert", "<Del>", "insert.delete")

    -- Insert text.
    Core.Commands.register("insert.insert", {
        metadata = { modifies = true },
        run = function(key_str)
            local viewport = Cini.workspace.viewport
            local doc = viewport.doc

            doc:insert(viewport.cursor:point(doc), key_str)
            viewport:move_cursor(Core.Cursor.right, Core.Utf8.count(key_str))

            return true
        end
    })
    Core.Keybinds.bind("insert", "<CatchAll>", "insert.insert")
end

return Insert
