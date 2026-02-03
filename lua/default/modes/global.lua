local Global = {}

local function apply_motion(motion, args, action)
    local viewport = Cini.workspace.viewport
    local doc = viewport.doc
    local start = doc.point

    motion(viewport.cursor, doc, table.unpack(args or {}))
    doc:set_point(viewport.cursor:point(doc))
    local stop = doc.point

    if stop < start then start, stop = stop, start end

    viewport:move_cursor(function(c, d, _) c:move_to(d, start) end, 0)

    action(doc, start, stop)
end

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

    Core.Keybinds.bind("global", "<C-q> <C-q>", function()
        Cini.workspace:close_split()
    end)

    Core.Keybinds.bind("global", "<C-g>", function()
        Cini.workspace.viewport:toggle_gutter()
    end)
    Core.Keybinds.bind("global", "<C-m>", function()
        Cini.workspace.viewport:toggle_mode_line()
    end)

    Core.Keybinds.bind("global", "h", function()
        Cini.workspace.viewport:move_cursor(Core.Cursor.left, 1)
    end)
    Core.Keybinds.bind("global", "j", function()
        Cini.workspace.viewport:move_cursor(Core.Cursor.down, 1)
    end)
    Core.Keybinds.bind("global", "k", function()
        Cini.workspace.viewport:move_cursor(Core.Cursor.up, 1)
    end)
    Core.Keybinds.bind("global", "l", function()
        Cini.workspace.viewport:move_cursor(Core.Cursor.right, 1)
    end)
    Core.Keybinds.bind("global", "<Left>", function()
        Cini.workspace.viewport:move_cursor(Core.Cursor.left, 1)
    end)
    Core.Keybinds.bind("global", "<Down>", function()
        Cini.workspace.viewport:move_cursor(Core.Cursor.down, 1)
    end)
    Core.Keybinds.bind("global", "<Up>", function()
        Cini.workspace.viewport:move_cursor(Core.Cursor.up, 1)
    end)
    Core.Keybinds.bind("global", "<Right>", function()
        Cini.workspace.viewport:move_cursor(Core.Cursor.right, 1)
    end)
    Core.Keybinds.bind("global", "<", function()
        Cini.workspace.viewport:move_cursor(function(cur, doc, _) cur:_jump_to_beginning_of_line(doc) end, 1)
    end)
    Core.Keybinds.bind("global", ">", function()
        Cini.workspace.viewport:move_cursor(function(cur, doc, _) cur:_jump_to_end_of_line(doc) end, 1)
    end)
    Core.Keybinds.bind("global", "<S-g>", function()
        Cini.workspace.viewport:move_cursor(function(cur, doc, _) cur:_jump_to_beginning_of_file(doc) end, 1)
    end)
    Core.Keybinds.bind("global", "g", function()
        Cini.workspace.viewport:move_cursor(function(cur, doc, _) cur:_jump_to_end_of_file(doc) end, 1)
    end)
    Core.Keybinds.bind("global", "w", function()
        Cini.workspace.viewport:move_cursor(Core.Cursor._next_word, 1)
    end)
    Core.Keybinds.bind("global", "<S-w>", function()
        Cini.workspace.viewport:move_cursor(Core.Cursor._next_word_end, 1)
    end)
    Core.Keybinds.bind("global", "b", function()
        Cini.workspace.viewport:move_cursor(Core.Cursor._prev_word, 1)
    end)
    Core.Keybinds.bind("global", "<S-b>", function()
        Cini.workspace.viewport:move_cursor(Core.Cursor._prev_word_end, 1)
    end)
    Core.Keybinds.bind("global", "s", function()
        Cini.workspace.viewport:move_cursor(Core.Cursor._next_whitespace, 1)
    end)
    Core.Keybinds.bind("global", "<S-s>", function()
        Cini.workspace.viewport:move_cursor(Core.Cursor._prev_whitespace, 1)
    end)
    Core.Keybinds.bind("global", "}", function()
        Cini.workspace.viewport:move_cursor(Core.Cursor._next_empty_line, 1)
    end)
    Core.Keybinds.bind("global", "{", function()
        Cini.workspace.viewport:move_cursor(Core.Cursor._prev_empty_line, 1)
    end)
    Core.Keybinds.bind("global", ".", function()
        Cini.workspace.viewport:move_cursor(function(cur, doc, _) cur:_jump_to_matching_opposite(doc) end, 1)
    end)

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

    local motions = {
        ["w"] = { func = Core.Cursor._next_word, args = { 1 } },
        ["b"] = { func = Core.Cursor._prev_word, args = { 1 } },
        ["<S-w>"] = { func = Core.Cursor._next_word_end, args = { 1 } },
        ["<S-b>"] = { func = Core.Cursor._prev_word_end, args = { 1 } },
        ["h"] = { func = Core.Cursor.left, args = { 1 } },
        ["l"] = { func = Core.Cursor.right, args = { 1 } },
        [">"] = { func = function(c, d) c:_jump_to_end_of_line(d) end, args = {} },
        ["<"] = { func = function(c, d) c:_jump_to_beginning_of_line(d) end, args = {} },
        ["<S-g>"] = { func = function(c, d) c:_jump_to_beginning_of_file(d) end, args = {} },
        ["g"] = { func = function(c, d) c:_jump_to_end_of_file(d) end, args = {} },
    }

    for key, motion in pairs(motions) do
        Core.Keybinds.bind("global", "d " .. key, function()
            apply_motion(motion.func, motion.args, function(doc, start, stop)
                doc:remove(start, stop)
            end)
        end)
    end
    Core.Keybinds.bind("global", "d d", function()
        local doc = Cini.workspace.viewport.doc
        local cursor = Cini.workspace.viewport.cursor

        cursor:_jump_to_beginning_of_line(doc)
        local start = cursor:point(doc)

        cursor:down(doc, 1)
        cursor:_jump_to_beginning_of_line(doc)
        local stop = cursor:point(doc)

        if start == stop then
            cursor:_jump_to_end_of_line(doc)
            stop = cursor:point(doc)
        end

        if start ~= stop then
            doc:remove(start, stop)
        end

        cursor:up(doc, 1)
    end)

    for key, motion in pairs(motions) do
        Core.Keybinds.bind("global", "c " .. key, function()
            apply_motion(motion.func, motion.args, function(doc, start, stop)
                doc:remove(start, stop)
                Core.Modes.add_minor_mode(doc, "insert")
            end)
        end)
    end
    Core.Keybinds.bind("global", "c c", function()
        local doc = Cini.workspace.viewport.doc
        local cursor = Cini.workspace.viewport.cursor

        cursor:_jump_to_beginning_of_line(doc)
        local start = cursor:point(doc)
        cursor:_jump_to_end_of_line(doc)
        local stop = cursor:point(doc)

        doc:remove(start, stop)

        Cini.workspace.viewport:move_cursor(function(c, d, _) c:move_to(d, start) end, 0)
        Core.Modes.add_minor_mode(doc, "insert")
    end)

    Core.Keybinds.bind("global", "r <CatchAll>", function(key)
        local doc = Cini.workspace.viewport.doc

        local char = doc:slice(doc.point, doc.point + 1)
        if char == "\n" then return true end

        doc:replace(doc.point, doc.point + Core.Utf8.len(char), key)
        return true
    end)

    Core.Keybinds.bind("global", "<C-n>", function()
        Cini.workspace.viewport:change_document(Cini:create_document(nil))
    end)

    Core.Keybinds.bind("global", "<C-s>", function()
        local doc = Cini.workspace.viewport.doc

        Core.Prompt.run("Save: ", doc.path, function(input)
            if input ~= "" then
                doc:save(input)
            else
                doc:save(nil)
            end

            Cini:set_status_message("Saved file...", "info_message", 0, false)
        end)
    end)

    Core.Keybinds.bind("global", "<C-o>", function()
        Core.Prompt.run("Open: ", nil, function(input)
            Cini.workspace.viewport:change_document(Cini:create_document(input ~= "" and input or nil))
        end)
    end)
end

return Global
