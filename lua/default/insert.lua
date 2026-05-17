local Insert = {}

function Insert.setup()
    -- Modes.
    Core.Modes.register_mode({
        name = "insert",
        cursor_style = Core.CursorStyle.BlinkingBar,
        metadata = {}
    })

    -- Mode Line.
    Core.ModeLine.register_indicator("insert", {
        run = function(_) return { { text = "[INS]", face = "selection.selection" } } end
    })

    -- Hooks.
    Core.Hooks.add("motion::registered", 50, function(name, motion)
        --- @cast name string
        --- @cast motion Core.Motion

        -- Commands.
        Core.Commands.register("global.change_" .. name, {
            metadata = {
                modifies = true,
                synopsis = "Change " .. name:gsub("_", " "),
                description = "Deletes text covered by the " .. name:gsub("_", " ") .. " motion and enters insert mode."
            },
            run = function()
                local view = Cini.workspace.viewport.view
                view.doc:begin_transaction(view.cur:point(view))

                Core.Motions.apply(motion, 1, function(doc_view, start, stop)
                    doc_view.doc:remove(start, stop)
                    Core.Modes.add_minor_mode(doc_view, "insert")

                    return start - stop
                end)

                -- The transaction isn't ended since we are in insert mode afterwards.
            end
        })

        -- Keybinds.
        for _, seq in ipairs(motion.sequences) do
            Core.Keybinds.bind("global", "c " .. seq, "global.change_" .. name)
        end
    end)

    -- Commands.
    Core.Commands.register("global.insert_mode", {
        metadata = {
            modifies = true,
            synopsis = "Enter insert mode",
            description = "Enters insert mode at the current cursor position."
        },
        run = function()
            local view = Cini.workspace.viewport.view
            view.doc:begin_transaction(view.cur:point(view))

            Core.Modes.add_minor_mode(view, "insert")
        end
    })
    Core.Commands.register("global.insert_mode_after", {
        metadata = {
            modifies = true,
            synopsis = "Insert after cursor",
            description = "Moves the cursor one character to the right and enters insert mode."
        },
        run = function()
            local view = Cini.workspace.viewport.view
            view.doc:begin_transaction(view.cur:point(view))

            view:move_cursor(Core.Cursor.right, 1)
            Core.Modes.add_minor_mode(view, "insert")
        end
    })
    Core.Commands.register("global.insert_mode_end_of_line", {
        metadata = {
            modifies = true,
            synopsis = "Insert at end of line",
            description = "Moves the cursor to the end of the current line and enters insert mode."
        },
        run = function()
            local view = Cini.workspace.viewport.view
            view.doc:begin_transaction(view.cur:point(view))

            view:move_cursor(function(c, v, _) c:_jump_to_end_of_line(v) end, 1)
            Core.Modes.add_minor_mode(view, "insert")
        end
    })
    Core.Commands.register("global.insert_newline_below", {
        metadata = {
            modifies = true,
            synopsis = "Insert newline below",
            description = "Inserts a new empty line below the current one and enters insert mode."
        },
        run = function()
            local view = Cini.workspace.viewport.view

            view.doc:begin_transaction(view.cur:point(view))

            view:move_cursor(function(c, v, _) c:_jump_to_end_of_line(v) end, 1)
            view.doc:insert(view.cur:point(view), "\n")
            view:move_cursor(Core.Cursor.right, 1)

            Core.Modes.add_minor_mode(view, "insert")
        end
    })
    Core.Commands.register("global.insert_newline_above", {
        metadata = {
            modifies = true,
            synopsis = "Insert newline above",
            description = "Inserts a new empty line above the current one and enters insert mode."
        },
        run = function()
            local view = Cini.workspace.viewport.view

            view.doc:begin_transaction(view.cur:point(view))

            view:move_cursor(function(c, v, _) c:_jump_to_beginning_of_line(v) end, 1)
            view.doc:insert(view.cur:point(view), "\n")
            view:move_cursor(function(c, v, _) c:_jump_to_beginning_of_line(v) end, 1)

            Core.Modes.add_minor_mode(view, "insert")
        end
    })

    Core.Commands.register("global.change_line", {
        metadata = {
            modifies = true,
            synopsis = "Change line",
            description = "Deletes the contents of the current line and enters insert mode."
        },
        run = function()
            local view = Cini.workspace.viewport.view
            local doc = view.doc
            local cur = view.cur
            local start = doc:line_begin_byte(cur.row)

            doc:begin_transaction(cur:point(view))

            cur:_jump_to_end_of_line(doc)
            local stop = cur:point(view)

            doc:remove(start, stop)

            view:move_cursor(function(c, v, _) c:move_to(v, start) end, 0)
            Core.Modes.add_minor_mode(view, "insert")

            -- The transaction isn't ended since we are in insert mode afterwards.
        end
    })

    Core.Commands.register("insert.exit", {
        metadata = {
            synopsis = "Exit insert mode",
            description = "Commits the current transaction, exits insert mode, and returns to normal mode."
        },
        run = function()
            local view = Cini.workspace.viewport.view
            view.doc:end_transaction(view.cur:point(view))

            Core.Modes.remove_minor_mode(view, "insert")
        end
    })

    Core.Commands.register("insert.move_left", {
        metadata = {
            synopsis = "Move left",
            description = "Moves the cursor one character to the left."
        },
        run = function() Cini.workspace.viewport.view:move_cursor(Core.Cursor.left, 1) end
    })
    Core.Commands.register("insert.move_prev_word", {
        metadata = {
            synopsis = "Move to previous word",
            description = "Moves the cursor to the beginning of the previous word."
        },
        run = function() Cini.workspace.viewport.view:move_cursor(Core.Cursor._prev_word, 1) end
    })
    Core.Commands.register("insert.move_right", {
        metadata = {
            synopsis = "Move right",
            description = "Moves the cursor one character to the right."
        },
        run = function() Cini.workspace.viewport.view:move_cursor(Core.Cursor.right, 1) end
    })
    Core.Commands.register("insert.move_next_word", {
        metadata = {
            synopsis = "Move to next word",
            description = "Moves the cursor to the beginning of the next word."
        },
        run = function() Cini.workspace.viewport.view:move_cursor(Core.Cursor._next_word, 1) end
    })
    Core.Commands.register("insert.move_up", {
        metadata = {
            synopsis = "Move up",
            description = "Moves the cursor up one line."
        },
        run = function() Cini.workspace.viewport.view:move_cursor(Core.Cursor.up, 1) end
    })
    Core.Commands.register("insert.move_down", {
        metadata = {
            synopsis = "Move down",
            description = "Moves the cursor down one line."
        },
        run = function() Cini.workspace.viewport.view:move_cursor(Core.Cursor.down, 1) end
    })

    Core.Commands.register("insert.space", {
        metadata = {
            modifies = true,
            synopsis = "Insert space",
            description = "Inserts a space character and breaks the undo transaction history."
        },
        run = function()
            local view = Cini.workspace.viewport.view

            view.doc:insert(view.cur:point(view), " ")
            view:move_cursor(Core.Cursor.right, 1)

            -- End transaction on whitespace.
            local pos = view.cur:point(view)
            view.doc:end_transaction(pos)
            view.doc:begin_transaction(pos)
        end
    })
    Core.Commands.register("insert.enter", {
        metadata = {
            modifies = true,
            synopsis = "Insert newline",
            description = "Inserts a newline character and breaks the undo transaction history."
        },
        run = function()
            local view = Cini.workspace.viewport.view

            view.doc:insert(view.cur:point(view), "\n")
            view:move_cursor(Core.Cursor.down, 1)
            view:move_cursor(function(c, v, _) c:_jump_to_beginning_of_line(v) end, 1)

            -- End transaction on whitespace.
            local pos = view.cur:point(view)
            view.doc:end_transaction(pos)
            view.doc:begin_transaction(pos)
        end
    })
    Core.Commands.register("insert.tab", {
        metadata = {
            modifies = true,
            synopsis = "Insert tab padding",
            description = "Inserts spaces to align with the next tab stop and breaks the undo transaction history."
        },
        run = function()
            local view = Cini.workspace.viewport.view

            local tab_width = Core.Util.tab_width(view.properties["tab_width"] or 4, view.cur.col)
            view.doc:insert(view.cur:point(view), (" "):rep(tab_width))
            view:move_cursor(Core.Cursor.right, tab_width)

            -- End transaction on whitespace.
            local pos = view.cur:point(view)
            view.doc:end_transaction(pos)
            view.doc:begin_transaction(pos)
        end
    })

    Core.Commands.register("insert.backspace", {
        metadata = {
            modifies = true,
            synopsis = "Backspace character",
            description = "Deletes the character immediately preceding the cursor."
        },
        run = function()
            local view = Cini.workspace.viewport.view
            local point = view.cur:point(view)

            if point ~= 0 and view:move_cursor(Core.Cursor.left, 1) then
                view.doc:remove(view.cur:point(view), point)
            end
        end
    })
    Core.Commands.register("insert.delete", {
        metadata = {
            modifies = true,
            synopsis = "Delete character",
            description = "Deletes the character directly under the cursor."
        },
        run = function()
            local view = Cini.workspace.viewport.view
            local point = view.cur:point(view)

            if point ~= view.doc.size then
                view.doc:remove(point, point + Core.Utf8.len(view.doc:slice(point, point + 1)))
            end
        end
    })

    Core.Commands.register("insert.delete_prev_word", {
        metadata = {
            modifies = true,
            synopsis = "Delete previous word",
            description = "Deletes from the cursor to the beginning of the previous word."
        },
        run = function()
            local view = Cini.workspace.viewport.view
            local point = view.cur:point(view)

            if point ~= 0 and view:move_cursor(Core.Cursor._prev_word, 1) then
                view.doc:remove(view.cur:point(view), point)
            end
        end
    })
    Core.Commands.register("insert.delete_next_word", {
        metadata = {
            modifies = true,
            synopsis = "Delete next word",
            description = "Deletes from the cursor to the beginning of the next word."
        },
        run = function()
            local view = Cini.workspace.viewport.view
            local point = view.cur:point(view)

            view:move_cursor(Core.Cursor._next_word, 1)
            local new_point = view.cur:point(view)

            if point ~= new_point then
                -- Move back to the initial position.
                view:move_cursor(function(c, v, _) c:move_to(v, point) end, 0)
                view.doc:remove(point, new_point)
            end
        end
    })

    Core.Commands.register("insert.insert", {
        metadata = {
            modifies = true,
            synopsis = "Insert character",
            description = "Inserts the typed character into the document."
        },
        run = function(key_str)
            -- Don't insert modifier keys (e.g. <S-Esc>).
            if key_str:match("^<.*>$") then return true end

            local view = Cini.workspace.viewport.view
            view.doc:insert(view.cur:point(view), key_str)
            view:move_cursor(Core.Cursor.right, Core.Utf8.count(key_str))

            return true
        end
    })

    -- Keybinds.
    Core.Keybinds.bind("global", "i", "global.insert_mode")
    Core.Keybinds.bind("global", "a", "global.insert_mode_after")
    Core.Keybinds.bind("global", "A", "global.insert_mode_end_of_line")
    Core.Keybinds.bind("global", "o", "global.insert_newline_below")
    Core.Keybinds.bind("global", "O", "global.insert_newline_above")

    Core.Keybinds.bind("global", "c c", "global.change_line")

    Core.Keybinds.bind("insert", "<Esc>", "insert.exit")

    Core.Keybinds.bind("insert", "<Left>", "insert.move_left")
    Core.Keybinds.bind("insert", "<M-Left>", "insert.move_prev_word")
    Core.Keybinds.bind("insert", "<Right>", "insert.move_right")
    Core.Keybinds.bind("insert", "<M-Right>", "insert.move_next_word")
    Core.Keybinds.bind("insert", "<Up>", "insert.move_up")
    Core.Keybinds.bind("insert", "<Down>", "insert.move_down")

    Core.Keybinds.bind("insert", "<Space>", "insert.space")
    Core.Keybinds.bind("insert", "<Enter>", "insert.enter")
    Core.Keybinds.bind("insert", "<Tab>", "insert.tab")
    Core.Keybinds.bind("insert", "<Bspc>", "insert.backspace")
    Core.Keybinds.bind("insert", "<Del>", "insert.delete")

    Core.Keybinds.bind("insert", "<M-Bspc>", "insert.delete_prev_word")
    Core.Keybinds.bind("insert", "<M-Del>", "insert.delete_next_word")

    Core.Keybinds.bind("insert", "<CatchAll>", "insert.insert")
end

function Insert.init() end

return Insert
