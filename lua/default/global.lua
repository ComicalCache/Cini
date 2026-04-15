local Global = {}

function Global.setup()
    -- Faces.
    Core.Faces.register_face("default", Core.Face({ fg = Core.Rgb(172, 178, 190), bg = Core.Rgb(41, 44, 51) }))
    Core.Faces.register_face("gutter", Core.Face({ fg = Core.Rgb(101, 103, 105), bg = Core.Rgb(36, 40, 46) }))
    Core.Faces.register_face("replacement", Core.Face({ bg = Core.Rgb(109, 110, 109) }))
    Core.Faces.register_face("current_line", Core.Face({ bg = Core.Rgb(50, 54, 60) }))
    Core.Faces.register_face("mode_line", Core.Face({ fg = Core.Rgb(172, 178, 190), bg = Core.Rgb(59, 61, 66) }))

    local default = Core.Faces.get_face("default") or {}
    Core.Faces.register_face("error_message", Core.Face({ fg = Core.Rgb(224, 108, 117), bg = default.bg }))
    Core.Faces.register_face("info_message", Core.Face({ fg = Core.Rgb(97, 175, 239), bg = default.bg }))

    Core.Faces.register_face("ws", Core.Face({ fg = Core.Rgb(68, 71, 79) }))
    Core.Faces.register_face("nl", Core.Face({ fg = Core.Rgb(68, 71, 79) }))
    Core.Faces.register_face("tab", Core.Face({ fg = Core.Rgb(68, 71, 79) }))

    -- Modes.
    Core.Modes.register_mode({
        name = "error_message",
        faces = { default = "error_message" }
    })
    Core.Modes.register_mode({
        name = "info_message",
        faces = { default = "info_message" }
    })

    -- Hooks.
    Core.Hooks.add("cini::startup", 10, function()
        if Cini.cli_args.mode then Core.Modes.set_major_mode(Cini.workspace.viewport.view.doc, Cini.cli_args.mode) end
    end)

    Core.Hooks.add("cursor::after-move", 10, function(view, _)
        --- @cast view Core.DocumentView

        local viewport = Cini.workspace.viewport
        if viewport.view == view then viewport:adjust() end
    end)

    Core.Hooks.add("document_view::created", 10, function(view)
        --- @cast view Core.DocumentView

        view.properties["ws"] = "·"
        view.properties["nl"] = "⏎"
        view.properties["tab"] = "↦"
    end)

    Core.Hooks.add("document::loaded", 10, function(doc)
        --- @cast doc Core.Document

        doc.properties["loaded"] = true
    end)
    Core.Hooks.add("document::unloaded", 10, function(doc)
        --- @cast doc Core.Document

        doc.properties["loaded"] = false
    end)

    Core.Hooks.add("document::before-insert", 10, function(doc, _, _)
        --- @cast doc Core.Document

        for _, view in ipairs(doc:views()) do view.properties["tmp_point"] = view.cur:point(view) end
    end)
    Core.Hooks.add("document::before-remove", 10, function(doc, _, _)
        --- @cast doc Core.Document

        for _, view in ipairs(doc:views()) do view.properties["tmp_point"] = view.cur:point(view) end
    end)
    Core.Hooks.add("document::after-insert", 10, function(doc, start, len)
        --- @cast doc Core.Document
        --- @cast start integer
        --- @cast len integer

        for _, view in ipairs(doc:views()) do
            local offset = view.properties["tmp_point"]
            if offset > start then view.cur:move_to(view, offset + len) end
            view.properties["tmp_point"] = nil
        end

        if Cini.workspace.viewport.view.doc == doc then Cini.workspace.viewport:adjust() end
    end)
    Core.Hooks.add("document::after-remove", 10, function(doc, start, len)
        --- @cast doc Core.Document
        --- @cast start integer
        --- @cast len integer

        for _, view in ipairs(doc:views()) do
            local offset = view.properties["tmp_point"]

            if offset > start then
                if offset <= start + len then -- The cursor was inside the deleted range.
                    view.cur:move_to(view, start)
                else                          -- The cursor was after the deleted range.
                    view.cur:move_to(view, offset - len)
                end
            else
                view.cur:move_to(view, offset)
            end

            view.properties["tmp_point"] = nil
        end

        if Cini.workspace.viewport.view.doc == doc then Cini.workspace.viewport:adjust() end
    end)
    Core.Hooks.add("document::after-clear", 10, function(doc)
        --- @cast doc Core.Document

        for _, view in ipairs(doc:views()) do view.cur:move_to(view, 0) end
        if Cini.workspace.viewport.view.doc == doc then Cini.workspace.viewport:adjust() end
    end)

    Core.Hooks.add("document_view::loaded", 10, function(view)
        --- @cast view Core.DocumentView

        view.properties["loaded"] = true
    end)
    Core.Hooks.add("document_view::unloaded", 10, function(view)
        --- @cast view Core.DocumentView

        view.properties["loaded"] = false
    end)

    Core.Hooks.add("motion::registered", 10, function(name, motion)
        --- @cast name string
        --- @cast motion Core.Motion

        -- Commands.
        Core.Commands.register("global.move_" .. name, {
            metadata = {},
            run = function() Cini.workspace.viewport.view:move_cursor(motion.run, 1) end
        })
        Core.Commands.register("global.delete_" .. name, {
            metadata = { modifies = true },
            run = function()
                local view = Cini.workspace.viewport.view
                view.doc:begin_transaction(view.cur:point(view))

                Core.Motions.apply(motion, 1, function(doc_view, start, stop)
                    doc_view.doc:remove(start, stop)
                    return start - stop
                end)

                view.doc:end_transaction(view.cur:point(view))
            end
        })
        Core.Commands.register("global.yank_" .. name, {
            metadata = {},
            run = function()
                Core.Motions.apply(motion, 1, function(view, start, stop)
                    Core.Clipboard.set_system_clipboard(view.doc:slice(start, stop))

                    return 0
                end)
            end
        })

        -- Keybinds.
        Core.Keybinds.bind("global", motion.sequence, "global.move_" .. name)
        Core.Keybinds.bind("global", "d " .. motion.sequence, "global.delete_" .. name)
        Core.Keybinds.bind("global", "y " .. motion.sequence, "global.yank_" .. name)
    end)

    -- Commands.
    Core.Commands.register("global.undo", {
        metadata = { modifies = true },
        run = function()
            local viewport = Cini.workspace.viewport
            local view = viewport.view

            local point = view.doc:undo()
            if point then
                view.cur:move_to(view, point)
                viewport:adjust()
            end
        end
    })
    Core.Commands.register("global.redo", {
        metadata = { modifies = true },
        run = function()
            local viewport = Cini.workspace.viewport
            local view = viewport.view

            local point = view.doc:redo()
            if point then
                view.cur:move_to(view, point)
                viewport:adjust()
            end
        end
    })

    Core.Commands.register("global.close_split", {
        metadata = {},
        run = function() if Cini.workspace:close_split() then Core.Quit.safe_quit() end end
    })

    Core.Commands.register("global.scroll_left",
        { metadata = {}, run = function() Cini.workspace.viewport:scroll_left(1) end })
    Core.Commands.register("global.scroll_down",
        { metadata = {}, run = function() Cini.workspace.viewport:scroll_down(1) end })
    Core.Commands.register("global.scroll_up",
        { metadata = {}, run = function() Cini.workspace.viewport:scroll_up(1) end })
    Core.Commands.register("global.scroll_right",
        { metadata = {}, run = function() Cini.workspace.viewport:scroll_right(1) end })

    Core.Commands.register("global.toggle_gutter", {
        metadata = {},
        run = function()
            local viewport = Cini.workspace.viewport

            viewport.view.gutter = not viewport.view.gutter
            viewport:adjust()
        end
    })
    Core.Commands.register("global.toggle_mode_line", {
        metadata = {},
        run = function()
            local viewport = Cini.workspace.viewport

            viewport.view.mode_line = not viewport.view.mode_line
            viewport:adjust()
        end
    })

    Core.Commands.register("global.split_vertical",
        { metadata = {}, run = function() Cini.workspace:split_vertical(0.5) end })
    Core.Commands.register("global.split_horizontal",
        { metadata = {}, run = function() Cini.workspace:split_horizontal(0.5) end })
    Core.Commands.register("global.resize_split_inc",
        { metadata = {}, run = function() Cini.workspace:resize_split(0.05) end })
    Core.Commands.register("global.resize_split_dec",
        { metadata = {}, run = function() Cini.workspace:resize_split(-0.05) end })

    Core.Commands.register("global.navigate_split_left",
        { metadata = {}, run = function() Cini.workspace:navigate_split(Core.Direction.Left) end })
    Core.Commands.register("global.navigate_split_down",
        { metadata = {}, run = function() Cini.workspace:navigate_split(Core.Direction.Down) end })
    Core.Commands.register("global.navigate_split_up",
        { metadata = {}, run = function() Cini.workspace:navigate_split(Core.Direction.Up) end })
    Core.Commands.register("global.navigate_split_right",
        { metadata = {}, run = function() Cini.workspace:navigate_split(Core.Direction.Right) end })

    Core.Commands.register("global.delete_char", {
        metadata = { modifies = true },
        run = function()
            local view = Cini.workspace.viewport.view
            view.doc:begin_transaction(view.cur:point(view))

            local motion = Core.Motions.get_motion("right") or {}
            Core.Motions.apply(motion, 1, function(doc_view, start, stop)
                doc_view.doc:remove(start, stop)
                return start - stop
            end)

            view.doc:end_transaction(view.cur:point(view))
        end
    })

    Core.Commands.register("global.delete_line", {
        metadata = { modifies = true },
        run = function()
            local view = Cini.workspace.viewport.view
            local doc = view.doc
            local cur = view.cur
            local start = doc:line_begin_byte(cur.row)

            doc:begin_transaction(cur:point(view))

            cur:down(view, 1)
            local stop = doc:line_begin_byte(cur.row)

            if start == stop then
                stop = doc:line_end_byte(cur.row)
                if cur.row > 0 then
                    start = start - 1
                    cur:up(view, 1)
                end
            else
                cur:up(view, 1)
            end

            if start ~= stop then doc:remove(start, stop) end

            doc:end_transaction(cur:point(view))
        end
    })
    Core.Commands.register("global.yank_line", {
        metadata = {},
        run = function()
            local view = Cini.workspace.viewport.view
            local start = view.doc:line_begin_byte(view.cur.row)
            local stop = view.doc:line_end_byte(view.cur.row)

            if start ~= stop then Core.Clipboard.set_system_clipboard(view.doc:slice(start, stop)) end
        end
    })

    Core.Commands.register("global.paste", {
        metadata = { modifies = true },
        run = function()
            local view = Cini.workspace.viewport.view

            view.doc:begin_transaction(view.cur:point(view))
            view.doc:insert(view.cur:point(view), Core.Clipboard.get_system_clipboard())
            view.doc:end_transaction(view.cur:point(view))
        end
    })

    Core.Commands.register("global.replace_char", {
        metadata = { modifies = true },
        run = function(key)
            local view = Cini.workspace.viewport.view
            local pos = view.cur:point(view)

            local char = view.doc:slice(pos, pos + 1)
            if char == "\n" then return true end

            view.doc:begin_transaction(pos)
            view.doc:replace(pos, pos + Core.Utf8.len(char), key)
            -- Keep the cursor on the same character that got replaced.
            view:move_cursor(function(c, v, _) c:move_to(v, pos) end, 0)
            view.doc:end_transaction(view.cur:point(view))

            return true
        end
    })

    Core.Commands.register("global.new_document", {
        metadata = { changes_view = true },
        run = function()
            Cini.workspace.viewport:change_document_view(Cini:create_document_view(Cini:create_document(nil)))
        end
    })
    Core.Commands.register("global.open_document", {
        metadata = { changes_view = true },
        run = function()
            local doc = Cini.workspace.viewport.view.doc
            local dir = nil

            if doc and doc.path then dir = doc.path:match("^(.*[/\\])") end

            if not dir or dir == "" then
                local pwd = os.getenv("PWD")
                if pwd then dir = pwd .. "/" end
            end

            Core.Prompt.run("Open: ", dir, function(input)
                Cini.workspace.viewport:change_document_view(
                    Cini:create_document_view(Cini:create_document(input ~= "" and input or nil)))
            end)
        end
    })
    Core.Commands.register("global.save_document", {
        metadata = { modifies = true },
        run = function()
            local doc = Cini.workspace.viewport.view.doc
            Core.Prompt.run("Save: ", doc.path, function(input)
                if input ~= "" then doc:save(input) else doc:save(nil) end
                Cini:set_status_message("Saved file", "info_message", 0, false)
            end)
        end
    })

    Core.Commands.register("global.health", {
        metadata = {},
        run = function()
            collectgarbage()

            local stats = Cini:debug_stats()
            local msg = ("[Health] Docs: %d (%d tracked) | Views: %d (%d tracked) | Viewports: %d"):format(
                stats.document_instances, stats.documents, stats.document_view_instances, stats.document_views,
                stats.viewport_instances)

            Cini:set_status_message(msg, "info_message", 0, false)
        end
    })

    -- Keybinds.
    Core.Keybinds.bind("global", "u", "global.undo")
    Core.Keybinds.bind("global", "U", "global.redo")

    Core.Keybinds.bind("global", "<C-q>", "global.close_split")

    Core.Keybinds.bind("global", "<S-h>", "global.scroll_left")
    Core.Keybinds.bind("global", "<S-j>", "global.scroll_down")
    Core.Keybinds.bind("global", "<S-k>", "global.scroll_up")
    Core.Keybinds.bind("global", "<S-l>", "global.scroll_right")

    Core.Keybinds.bind("global", "<C-w> <C-g>", "global.toggle_gutter")
    Core.Keybinds.bind("global", "<C-w> <C-m>", "global.toggle_mode_line")

    Core.Keybinds.bind("global", "<C-w> <S-v>", "global.split_vertical")
    Core.Keybinds.bind("global", "<C-w> <S-h>", "global.split_horizontal")
    Core.Keybinds.bind("global", "<C-w> +", "global.resize_split_inc")
    Core.Keybinds.bind("global", "<C-w> -", "global.resize_split_dec")

    Core.Keybinds.bind("global", "<C-w> h", "global.navigate_split_left")
    Core.Keybinds.bind("global", "<C-w> j", "global.navigate_split_down")
    Core.Keybinds.bind("global", "<C-w> k", "global.navigate_split_up")
    Core.Keybinds.bind("global", "<C-w> l", "global.navigate_split_right")

    Core.Keybinds.bind("global", "x", "global.delete_char")

    Core.Keybinds.bind("global", "d d", "global.delete_line")
    Core.Keybinds.bind("global", "y y", "global.yank_line")

    Core.Keybinds.bind("global", "p", "global.paste")

    Core.Keybinds.bind("global", "R <CatchAll>", "global.replace_char")

    Core.Keybinds.bind("global", "<C-n>", "global.new_document")
    Core.Keybinds.bind("global", "<C-o>", "global.open_document")
    Core.Keybinds.bind("global", "<C-s>", "global.save_document")

    Core.Keybinds.bind("global", "<S-Esc>", "global.health")
end

function Global.init()
    -- Motions.
    Core.Motions.register_motion("left",
        { sequence = "h", run = function(cur, view, n) cur:left(view, n) end })
    Core.Motions.register_motion("down",
        { sequence = "j", run = function(cur, view, n) cur:down(view, n) end })
    Core.Motions.register_motion("up",
        { sequence = "k", run = function(cur, view, n) cur:up(view, n) end })
    Core.Motions.register_motion("right",
        { sequence = "l", run = function(cur, view, n) cur:right(view, n) end })
    Core.Motions.register_motion("beginning_of_line",
        { sequence = "<", run = function(cur, view, _) cur:_jump_to_beginning_of_line(view) end })
    Core.Motions.register_motion("end_of_line",
        { sequence = ">", run = function(cur, view, _) cur:_jump_to_end_of_line(view) end })
    Core.Motions.register_motion("beginning_of_file",
        { sequence = "<S-g>", run = function(cur, view, _) cur:_jump_to_beginning_of_file(view) end })
    Core.Motions.register_motion("end_of_file",
        { sequence = "g", run = function(cur, view, _) cur:_jump_to_end_of_file(view) end })
    Core.Motions.register_motion("next_word",
        { sequence = "w", run = function(cur, view, n) cur:_next_word(view, n) end })
    Core.Motions.register_motion("next_word_end",
        { sequence = "<S-w>", run = function(cur, view, n) cur:_next_word_end(view, n) end })
    Core.Motions.register_motion("prev_word",
        { sequence = "b", run = function(cur, view, n) cur:_prev_word(view, n) end })
    Core.Motions.register_motion("prev_word_end",
        { sequence = "<S-b>", run = function(cur, view, n) cur:_prev_word_end(view, n) end })
    Core.Motions.register_motion("next_whitespace",
        { sequence = "s", run = function(cur, view, n) cur:_next_whitespace(view, n) end })
    Core.Motions.register_motion("prev_whitespace",
        { sequence = "<S-s>", run = function(cur, view, n) cur:_prev_whitespace(view, n) end })
    Core.Motions.register_motion("next_empty_line",
        { sequence = "}", run = function(cur, view, n) cur:_next_empty_line(view, n) end })
    Core.Motions.register_motion("prev_empty_line",
        { sequence = "{", run = function(cur, view, n) cur:_prev_empty_line(view, n) end })
    Core.Motions.register_motion("opposite",
        { sequence = ".", run = function(cur, view, _) cur:_jump_to_matching_opposite(view) end })
end

return Global
