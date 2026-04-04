local Global = {}

function Global.init()
    Core.Faces.register_face("default", Core.Face({ fg = Core.Rgb(172, 178, 190), bg = Core.Rgb(41, 44, 51) }))
    Core.Faces.register_face("gutter", Core.Face({ fg = Core.Rgb(101, 103, 105), bg = Core.Rgb(36, 40, 46) }))
    Core.Faces.register_face("replacement", Core.Face({ bg = Core.Rgb(109, 110, 109) }))
    Core.Faces.register_face("current_line", Core.Face({ bg = Core.Rgb(50, 54, 60) }))
    Core.Faces.register_face("mode_line", Core.Face({ fg = Core.Rgb(172, 178, 190), bg = Core.Rgb(59, 61, 66) }))
    Core.Faces.register_face("selection", Core.Face({ fg = Core.Rgb(41, 44, 51), bg = Core.Rgb(97, 175, 239) }))

    Core.Faces.register_face("ws", Core.Face({ fg = Core.Rgb(68, 71, 79) }))
    Core.Faces.register_face("nl", Core.Face({ fg = Core.Rgb(68, 71, 79) }))
    Core.Faces.register_face("tab", Core.Face({ fg = Core.Rgb(68, 71, 79) }))

    Core.Hooks.add("command::before-execute", 1, function(_, _) return true end)

    Core.Hooks.add("document::created", 1, function(doc)
        doc.properties["ws"] = "·"
        doc.properties["nl"] = "⏎"
        doc.properties["tab"] = "↦"
    end)

    Core.Hooks.add("document::loaded", 1, function(doc) doc.properties["loaded"] = true end)
    Core.Hooks.add("document::unloaded", 1, function(doc) doc.properties["loaded"] = false end)

    Core.Hooks.add("motion::registered", 1, function(name, motion)
        -- Move.
        Core.Commands.register("global.move_" .. name, {
            metadata = { modifies = false },
            run = function() Cini.workspace.viewport:move_cursor(motion.run, 1) end
        })
        Core.Keybinds.bind("global", motion.sequence, "global.move_" .. name)

        -- Delete.
        Core.Commands.register("global.delete_" .. name, {
            metadata = { modifies = true },
            run = function()
                local viewport = Cini.workspace.viewport
                viewport.doc:begin_transaction(viewport.cursor:point(viewport.doc))

                Core.Motions.apply(motion, 1, function(doc, start, stop)
                    doc:remove(start, stop)
                    return start - stop
                end)

                viewport.doc:end_transaction(viewport.cursor:point(viewport.doc))
            end
        })
        Core.Keybinds.bind("global", "d " .. motion.sequence, "global.delete_" .. name)

        -- Change.
        Core.Commands.register("global.change_" .. name, {
            metadata = { modifies = true },
            run = function()
                local viewport = Cini.workspace.viewport
                viewport.doc:begin_transaction(viewport.cursor:point(viewport.doc))

                Core.Motions.apply(motion, 1, function(doc, start, stop)
                    doc:remove(start, stop)
                    Core.Modes.add_minor_mode(doc, "insert")

                    return start - stop
                end)

                -- The transaction isn't ended since we are in insert mode afterwards.
            end
        })
        Core.Keybinds.bind("global", "c " .. motion.sequence, "global.change_" .. name)

        -- Yank.
        Core.Commands.register("global.yank_" .. name, {
            metadata = { modifies = false },
            run = function()
                Core.Motions.apply(motion, 1, function(doc, start, stop)
                    Core.Util.set_system_clipboard(doc:slice(start, stop))

                    return 0
                end)
            end
        })
        Core.Keybinds.bind("global", "y " .. motion.sequence, "global.yank_" .. name)
    end)

    Core.Motions.register_motion("left", { sequence = "h", run = Core.Cursor.left })
    Core.Motions.register_motion("down", { sequence = "j", run = Core.Cursor.down })
    Core.Motions.register_motion("up", { sequence = "k", run = Core.Cursor.up })
    Core.Motions.register_motion("right", { sequence = "l", run = Core.Cursor.right })
    Core.Motions.register_motion("beginning_of_line",
        { sequence = "<", run = function(cur, doc, _) cur:_jump_to_beginning_of_line(doc) end })
    Core.Motions.register_motion("end_of_line",
        { sequence = ">", run = function(cur, doc, _) cur:_jump_to_end_of_line(doc) end })
    Core.Motions.register_motion("beginning_of_file",
        { sequence = "<S-g>", run = function(cur, doc, _) cur:_jump_to_beginning_of_file(doc) end })
    Core.Motions.register_motion("end_of_file",
        { sequence = "g", run = function(cur, doc, _) cur:_jump_to_end_of_file(doc) end })
    Core.Motions.register_motion("next_word", { sequence = "w", run = Core.Cursor._next_word })
    Core.Motions.register_motion("next_word_end", { sequence = "<S-w>", run = Core.Cursor._next_word_end })
    Core.Motions.register_motion("prev_word", { sequence = "b", run = Core.Cursor._prev_word })
    Core.Motions.register_motion("prev_word_end", { sequence = "<S-b>", run = Core.Cursor._prev_word_end })
    Core.Motions.register_motion("next_whitespace", { sequence = "s", run = Core.Cursor._next_whitespace })
    Core.Motions.register_motion("prev_whitespace", { sequence = "<S-s>", run = Core.Cursor._prev_whitespace })
    Core.Motions.register_motion("next_empty_line", { sequence = "}", run = Core.Cursor._next_empty_line })
    Core.Motions.register_motion("prev_empty_line", { sequence = "{", run = Core.Cursor._prev_empty_line })
    Core.Motions.register_motion("opposite",
        { sequence = ".", run = function(cur, doc, _) cur:_jump_to_matching_opposite(doc) end })

    -- Undo/Redo.
    Core.Commands.register("global.undo", {
        metadata = { modifies = true },
        run = function()
            local viewport = Cini.workspace.viewport
            local point = viewport.doc:undo()
            if point then viewport.cursor:move_to(viewport.doc, point) end
        end
    })
    Core.Commands.register("global.redo", {
        metadata = { modifies = true },
        run = function()
            local viewport = Cini.workspace.viewport
            local point = viewport.doc:redo()
            if point then viewport.cursor:move_to(viewport.doc, point) end
        end
    })
    Core.Keybinds.bind("global", "u", "global.undo")
    Core.Keybinds.bind("global", "U", "global.redo")

    -- Close.
    Core.Commands.register("global.close_split", {
        metadata = { modifies = false },
        run = function() if Cini.workspace:close_split() then Global.safe_quit() end end
    })
    Core.Keybinds.bind("global", "<C-q>", "global.close_split")

    -- Viewport movement.
    Core.Commands.register("global.scroll_left",
        { metadata = { modifies = false }, run = function() Cini.workspace.viewport:scroll_left(1) end })
    Core.Commands.register("global.scroll_down",
        { metadata = { modifies = false }, run = function() Cini.workspace.viewport:scroll_down(1) end })
    Core.Commands.register("global.scroll_up",
        { metadata = { modifies = false }, run = function() Cini.workspace.viewport:scroll_up(1) end })
    Core.Commands.register("global.scroll_right",
        { metadata = { modifies = false }, run = function() Cini.workspace.viewport:scroll_right(1) end })
    Core.Keybinds.bind("global", "<S-h>", "global.scroll_left")
    Core.Keybinds.bind("global", "<S-j>", "global.scroll_down")
    Core.Keybinds.bind("global", "<S-k>", "global.scroll_up")
    Core.Keybinds.bind("global", "<S-l>", "global.scroll_right")

    -- Window decoration.
    Core.Commands.register("global.toggle_gutter",
        { metadata = { modifies = false }, run = function() Cini.workspace.viewport:toggle_gutter() end })
    Core.Commands.register("global.toggle_mode_line",
        { metadata = { modifies = false }, run = function() Cini.workspace.viewport:toggle_mode_line() end })
    Core.Keybinds.bind("global", "<C-w> <C-g>", "global.toggle_gutter")
    Core.Keybinds.bind("global", "<C-w> <C-m>", "global.toggle_mode_line")

    -- Window splits.
    Core.Commands.register("global.split_vertical",
        { metadata = { modifies = false }, run = function() Cini.workspace:split_vertical(0.5) end })
    Core.Commands.register("global.split_horizontal",
        { metadata = { modifies = false }, run = function() Cini.workspace:split_horizontal(0.5) end })
    Core.Commands.register("global.resize_split_inc",
        { metadata = { modifies = false }, run = function() Cini.workspace:resize_split(0.05) end })
    Core.Commands.register("global.resize_split_dec",
        { metadata = { modifies = false }, run = function() Cini.workspace:resize_split(-0.05) end })
    Core.Keybinds.bind("global", "<C-w> <S-v>", "global.split_vertical")
    Core.Keybinds.bind("global", "<C-w> <S-h>", "global.split_horizontal")
    Core.Keybinds.bind("global", "<C-w> +", "global.resize_split_inc")
    Core.Keybinds.bind("global", "<C-w> -", "global.resize_split_dec")

    Core.Commands.register("global.navigate_split_left",
        { metadata = { modifies = false }, run = function() Cini.workspace:navigate_split(Core.Direction.Left) end })
    Core.Commands.register("global.navigate_split_down",
        { metadata = { modifies = false }, run = function() Cini.workspace:navigate_split(Core.Direction.Down) end })
    Core.Commands.register("global.navigate_split_up",
        { metadata = { modifies = false }, run = function() Cini.workspace:navigate_split(Core.Direction.Up) end })
    Core.Commands.register("global.navigate_split_right",
        { metadata = { modifies = false }, run = function() Cini.workspace:navigate_split(Core.Direction.Right) end })
    Core.Keybinds.bind("global", "<C-w> h", "global.navigate_split_left")
    Core.Keybinds.bind("global", "<C-w> j", "global.navigate_split_down")
    Core.Keybinds.bind("global", "<C-w> k", "global.navigate_split_up")
    Core.Keybinds.bind("global", "<C-w> l", "global.navigate_split_right")

    -- Document management.
    Core.Commands.register("global.open_document_viewer",
        { metadata = { modifies = false }, run = function() Core.DocumentViewer.open() end })
    Core.Keybinds.bind("global", "<C-b>", "global.open_document_viewer")

    -- Delete character.
    Core.Commands.register("global.delete_char", {
        metadata = { modifies = true },
        run = function()
            local viewport = Cini.workspace.viewport
            viewport.doc:begin_transaction(viewport.cursor:point(viewport.doc))

            local motion = Core.Motions.get_motion("right") or {}
            Core.Motions.apply(motion, 1, function(doc, start, stop)
                doc:remove(start, stop)
                return start - stop
            end)

            viewport.doc:end_transaction(viewport.cursor:point(viewport.doc))
        end
    })
    Core.Keybinds.bind("global", "x", "global.delete_char")

    -- Line operations.
    Core.Commands.register("global.delete_line", {
        metadata = { modifies = true },
        run = function()
            local viewport = Cini.workspace.viewport
            local doc = viewport.doc
            local cursor = viewport.cursor
            local start = doc:line_begin_byte(cursor.row)

            doc:begin_transaction(viewport.cursor:point(doc))

            cursor:down(doc, 1)
            local stop = doc:line_begin_byte(cursor.row)

            if start == stop then
                stop = doc:line_end_byte(cursor.row)
                if cursor.row > 0 then
                    start = start - 1
                    cursor:up(doc, 1)
                end
            else
                cursor:up(doc, 1)
            end

            if start ~= stop then doc:remove(start, stop) end

            doc:end_transaction(cursor:point(doc))
        end
    })
    Core.Commands.register("global.change_line", {
        metadata = { modifies = true },
        run = function()
            local viewport = Cini.workspace.viewport
            local doc = viewport.doc
            local cursor = viewport.cursor
            local start = doc:line_begin_byte(cursor.row)

            doc:begin_transaction(cursor:point(doc))

            cursor:_jump_to_end_of_line(doc)
            local stop = cursor:point(doc)

            doc:remove(start, stop)

            Cini.workspace.viewport:move_cursor(function(c, d, _) c:move_to(d, start) end, 0)
            Core.Modes.add_minor_mode(doc, "insert")

            -- The transaction isn't ended since we are in insert mode afterwards.
        end
    })
    Core.Commands.register("global.yank_line", {
        metadata = { modifies = true },
        run = function()
            local doc = Cini.workspace.viewport.doc
            local cursor = Cini.workspace.viewport.cursor
            local start = doc:line_begin_byte(cursor.row)
            local stop = doc:line_end_byte(cursor.row)

            if start ~= stop then Core.Util.set_system_clipboard(doc:slice(start, stop)) end
        end
    })
    Core.Keybinds.bind("global", "d d", "global.delete_line")
    Core.Keybinds.bind("global", "c c", "global.change_line")
    Core.Keybinds.bind("global", "y y", "global.yank_line")

    -- Pase.
    Core.Commands.register("global.paste", {
        metadata = { modifies = true },
        run = function()
            local viewport = Cini.workspace.viewport
            local doc = viewport.doc

            doc:begin_transaction(viewport.cursor:point(doc))
            doc:insert(viewport.cursor:point(doc), Core.Util.get_system_clipboard())
            doc:end_transaction(viewport.cursor:point(doc))
        end
    })
    Core.Keybinds.bind("global", "p", "global.paste")

    -- Replace character.
    Core.Commands.register("global.replace_char", {
        metadata = { modifies = true },
        run = function(key)
            local viewport = Cini.workspace.viewport
            local doc = viewport.doc
            local pos = viewport.cursor:point(doc)

            local char = doc:slice(pos, pos + 1)
            if char == "\n" then return true end

            doc:begin_transaction(pos)
            doc:replace(pos, pos + Core.Utf8.len(char), key)
            -- Keep the cursor on the same character that got replaced.
            Cini.workspace.viewport:move_cursor(function(c, d, _) c:move_to(d, pos) end, 0)
            doc:end_transaction(viewport.cursor:point(doc))

            return true
        end
    })
    Core.Keybinds.bind("global", "r <CatchAll>", "global.replace_char")

    -- Document operations.
    Core.Commands.register("global.new_document", {
        metadata = { modifies = false },
        run = function() Cini.workspace.viewport:change_document(Cini:create_document(nil)) end
    })
    Core.Commands.register("global.open_document", {
        metadata = { modifies = false },
        run = function()
            Core.Prompt.run("Open: ", nil, function(input)
                Cini.workspace.viewport:change_document(Cini:create_document(input ~= "" and input or nil))
            end)
        end
    })
    Core.Commands.register("global.save_document", {
        metadata = { modifies = false },
        run = function()
            local doc = Cini.workspace.viewport.doc
            Core.Prompt.run("Save: ", doc.path, function(input)
                if input ~= "" then doc:save(input) else doc:save(nil) end
                Cini:set_status_message("Saved file", "info_message", 0, false)
            end)
        end
    })
    Core.Keybinds.bind("global", "<C-n>", "global.new_document")
    Core.Keybinds.bind("global", "<C-o>", "global.open_document")
    Core.Keybinds.bind("global", "<C-s>", "global.save_document")
end

--- Asks to discard unsaved changes before stopping the event loop and quitting Cini.
function Global.safe_quit()
    local count = 0
    local name = ""

    -- Check all documents for the modified flag
    for _, doc in ipairs(Cini.documents) do
        if doc.modified then
            count = count + 1
            name = doc.path or "Scratchpad"
        end
    end

    if count == 0 then
        Cini:quit()
    else
        local msg = ""
        if count == 1 then
            msg = string.format("%s has unsaved changes. Discard unsaved changes? (y/n) ", name)
        else
            msg = string.format("%d documents have unsaved changes. Discard unsaved changes? (y/n) ", count)
        end

        Core.Prompt.run(msg, nil, function(sel) if sel:lower() == "y" then Cini:quit() end end)
    end
end

return Global
