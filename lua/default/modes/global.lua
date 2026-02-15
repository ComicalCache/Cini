local Global = {}

function Global.init()
    Core.Faces.register_face("default", Core.Face({ fg = Core.Rgb(172, 178, 190), bg = Core.Rgb(41, 44, 51) }))
    Core.Faces.register_face("gutter", Core.Face({ fg = Core.Rgb(101, 103, 105), bg = Core.Rgb(36, 40, 46) }))
    Core.Faces.register_face("replacement", Core.Face({ bg = Core.Rgb(109, 110, 109) }))
    Core.Faces.register_face("mode_line", Core.Face({ fg = Core.Rgb(172, 178, 190), bg = Core.Rgb(59, 61, 66) }))

    Core.Hooks.add("document::created", function(doc)
        Core.Faces.register_face("ws", Core.Face({ fg = Core.Rgb(68, 71, 79) }))
        Core.Faces.register_face("nl", Core.Face({ fg = Core.Rgb(68, 71, 79) }))
        Core.Faces.register_face("tab", Core.Face({ fg = Core.Rgb(68, 71, 79), bg = Core.Rgb(181, 59, 59) }))

        doc.properties["ws"] = "·"
        doc.properties["nl"] = "⏎"
        doc.properties["tab"] = "↦"
    end)

    Core.Motions.register_motion("h", Core.Cursor.left)
    Core.Motions.register_motion("j", Core.Cursor.down)
    Core.Motions.register_motion("k", Core.Cursor.up)
    Core.Motions.register_motion("l", Core.Cursor.right)
    Core.Motions.register_motion("<", function(cur, doc, _) cur:_jump_to_beginning_of_line(doc) end)
    Core.Motions.register_motion(">", function(cur, doc, _) cur:_jump_to_end_of_line(doc) end)
    Core.Motions.register_motion("<S-g>", function(cur, doc, _) cur:_jump_to_beginning_of_file(doc) end)
    Core.Motions.register_motion("g", function(cur, doc, _) cur:_jump_to_end_of_file(doc) end)
    Core.Motions.register_motion("w", Core.Cursor._next_word)
    Core.Motions.register_motion("<S-w>", Core.Cursor._next_word_end)
    Core.Motions.register_motion("b", Core.Cursor._prev_word)
    Core.Motions.register_motion("<S-b>", Core.Cursor._prev_word_end)
    Core.Motions.register_motion("s", Core.Cursor._next_whitespace)
    Core.Motions.register_motion("<S-s>", Core.Cursor._prev_whitespace)
    Core.Motions.register_motion("}", Core.Cursor._next_empty_line)
    Core.Motions.register_motion("{", Core.Cursor._prev_empty_line)
    Core.Motions.register_motion(".", function(cur, doc, _) cur:_jump_to_matching_opposite(doc) end)

    -- Close.
    Core.Keybinds.bind("global", "<C-q> <C-q>", function()
        Cini.workspace:close_split()
    end)

    -- Movement.
    for key, motion in pairs(Core.Motions.motions) do
        Core.Keybinds.bind("global", key, function()
            Cini.workspace.viewport:move_cursor(motion, 1)
        end)
    end

    -- Viewport movement.
    Core.Keybinds.bind("global", "<S-h>", function()
        Cini.workspace.viewport:scroll_left(1)
    end)
    Core.Keybinds.bind("global", "<S-j>", function()
        Cini.workspace.viewport:scroll_down(1)
    end)
    Core.Keybinds.bind("global", "<S-k>", function()
        Cini.workspace.viewport:scroll_up(1)
    end)
    Core.Keybinds.bind("global", "<S-l>", function()
        Cini.workspace.viewport:scroll_right(1)
    end)

    -- Window decoration.
    Core.Keybinds.bind("global", "<C-w> <C-g>", function()
        Cini.workspace.viewport:toggle_gutter()
    end)
    Core.Keybinds.bind("global", "<C-w> <C-m>", function()
        Cini.workspace.viewport:toggle_mode_line()
    end)

    -- Window splits.
    Core.Keybinds.bind("global", "<C-w> <S-v>", function()
        Cini.workspace:split_vertical(0.5)
    end)
    Core.Keybinds.bind("global", "<C-w> <S-h>", function()
        Cini.workspace:split_horizontal(0.5)
    end)
    Core.Keybinds.bind("global", "<C-w> +", function()
        Cini.workspace:resize_split(0.05)
    end)
    Core.Keybinds.bind("global", "<C-w> -", function()
        Cini.workspace:resize_split(-0.05)
    end)

    Core.Keybinds.bind("global", "<C-w> h", function()
        Cini.workspace:navigate_split(Core.Direction.Left)
    end)
    Core.Keybinds.bind("global", "<C-w> j", function()
        Cini.workspace:navigate_split(Core.Direction.Down)
    end)
    Core.Keybinds.bind("global", "<C-w> k", function()
        Cini.workspace:navigate_split(Core.Direction.Up)
    end)
    Core.Keybinds.bind("global", "<C-w> l", function()
        Cini.workspace:navigate_split(Core.Direction.Right)
    end)

    -- Insert mode.
    Core.Keybinds.bind("global", "i", function()
        Core.Modes.add_minor_mode(Cini.workspace.viewport.doc, "insert")
    end)
    Core.Keybinds.bind("global", "a", function()
        Cini.workspace.viewport:move_cursor(Core.Cursor.right, 1)
        Core.Modes.add_minor_mode(Cini.workspace.viewport.doc, "insert")
    end)
    Core.Keybinds.bind("global", "A", function()
        Cini.workspace.viewport:move_cursor(function(cur, doc, _) cur:_jump_to_end_of_line(doc) end, 1)
        Core.Modes.add_minor_mode(Cini.workspace.viewport.doc, "insert")
    end)
    Core.Keybinds.bind("global", "o", function()
        local viewport = Cini.workspace.viewport
        local doc = viewport.doc

        viewport:move_cursor(function(cur, d, _) cur:_jump_to_end_of_line(d) end, 1)
        doc:insert(doc.point, "\n")
        viewport:move_cursor(Core.Cursor.right, 1)
        Core.Modes.add_minor_mode(doc, "insert")
    end)
    Core.Keybinds.bind("global", "O", function()
        local doc = Cini.workspace.viewport.doc

        Cini.workspace.viewport:move_cursor(function(cur, d, _) cur:_jump_to_beginning_of_line(d) end, 1)
        doc:insert(doc.point, "\n")
        Core.Modes.add_minor_mode(doc, "insert")
    end)

    for key, motion in pairs(Core.Motions.motions) do
        -- Delete motions.
        Core.Keybinds.bind("global", "d " .. key, function()
            Core.Motions.apply(motion, 1, function(doc, start, stop)
                doc:remove(start, stop)
            end)
        end)

        -- Change motions.
        Core.Keybinds.bind("global", "c " .. key, function()
            Core.Motions.apply(motion, 1, function(doc, start, stop)
                doc:remove(start, stop)
                Core.Modes.add_minor_mode(doc, "insert")
            end)
        end)
    end

    -- Delete line.
    Core.Keybinds.bind("global", "d d", function()
        local doc = Cini.workspace.viewport.doc
        local cursor = Cini.workspace.viewport.cursor

        local start = doc:line_begin_byte(cursor.row)
        cursor:down(doc, 1)
        local stop = doc:line_begin_byte(cursor.row)

        if start == stop then
            stop = doc:line_end_byte(cursor.row)
        end

        if start ~= stop then
            doc:remove(start, stop)
        end

        cursor:up(doc, 1)
    end)

    -- Change line.
    Core.Keybinds.bind("global", "c c", function()
        local doc = Cini.workspace.viewport.doc
        local cursor = Cini.workspace.viewport.cursor

        local start = doc:line_begin_byte(cursor.row)
        cursor:_jump_to_end_of_line(doc)
        local stop = cursor:point(doc)

        doc:remove(start, stop)

        Cini.workspace.viewport:move_cursor(function(c, d, _) c:move_to(d, start) end, 0)
        Core.Modes.add_minor_mode(doc, "insert")
    end)

    -- Replace character.
    Core.Keybinds.bind("global", "r <CatchAll>", function(key)
        local doc = Cini.workspace.viewport.doc

        local char = doc:slice(doc.point, doc.point + 1)
        if char == "\n" then return true end

        doc:replace(doc.point, doc.point + Core.Utf8.len(char), key)
        return true
    end)

    -- Document operations.
    Core.Keybinds.bind("global", "<C-n>", function()
        Cini.workspace.viewport:change_document(Cini:create_document(nil))
    end)
    Core.Keybinds.bind("global", "<C-o>", function()
        Core.Prompt.run("Open: ", nil, function(input)
            Cini.workspace.viewport:change_document(Cini:create_document(input ~= "" and input or nil))
        end)
    end)
    Core.Keybinds.bind("global", "<C-s>", function()
        local doc = Cini.workspace.viewport.doc

        Core.Prompt.run("Save: ", doc.path, function(input)
            if input ~= "" then
                doc:save(input)
            else
                doc:save(nil)
            end

            Cini:set_status_message("Saved file", "info_message", 0, false)
        end)
    end)
end

return Global
