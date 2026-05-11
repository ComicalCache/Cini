local Global = {}

function Global.setup()
    local ansi_colors = {
        black          = { 41, 44, 51 },
        red            = { 224, 108, 117 },
        green          = { 152, 195, 121 },
        yellow         = { 229, 192, 123 },
        blue           = { 97, 175, 239 },
        magenta        = { 198, 120, 221 },
        cyan           = { 86, 182, 194 },
        white          = { 172, 178, 190 },

        bright_black   = { 92, 99, 112 },
        bright_red     = { 235, 130, 138 },
        bright_green   = { 170, 207, 142 },
        bright_yellow  = { 240, 206, 145 },
        bright_blue    = { 120, 188, 242 },
        bright_magenta = { 210, 142, 230 },
        bright_cyan    = { 108, 194, 205 },
        bright_white   = { 200, 206, 218 },
    }

    -- Faces.
    for name, rgb in pairs(ansi_colors) do
        Core.Faces.register_face("ansi.fg." .. name, Core.Face({ fg = Core.Rgb(rgb[1], rgb[2], rgb[3]) }))
        Core.Faces.register_face("ansi.bg." .. name, Core.Face({ bg = Core.Rgb(rgb[1], rgb[2], rgb[3]) }))
    end

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

        local display_name = name:gsub("_", " ")

        -- Commands.
        Core.Commands.register("global.move_" .. name, {
            metadata = {
                synopsis = "Move " .. display_name,
                description = "Moves the cursor using the " .. display_name .. " motion."
            },
            run = function() Cini.workspace.viewport.view:move_cursor(motion.run, 1) end
        })
        Core.Commands.register("global.delete_" .. name, {
            metadata = {
                modifies = true,
                synopsis = "Delete " .. display_name,
                description = "Deletes the text covered by the " .. display_name .. " motion."
            },
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
            metadata = {
                synopsis = "Yank " .. display_name,
                description = "Copies the text covered by the " .. display_name .. " motion to the clipboard."
            },
            run = function()
                Core.Motions.apply(motion, 1, function(view, start, stop)
                    Core.Clipboard.set_system_clipboard(view.doc:slice(start, stop))

                    return 0
                end)
            end
        })

        -- Keybinds.
        for _, seq in ipairs(motion.sequences) do
            Core.Keybinds.bind("global", seq, "global.move_" .. name)
            Core.Keybinds.bind("global", "d " .. seq, "global.delete_" .. name)
            Core.Keybinds.bind("global", "y " .. seq, "global.yank_" .. name)
        end
    end)

    -- Commands.
    Core.Commands.register("global.command_palette", {
        metadata = {
            description = "Run an editor command",
            synopsis = "Runs an editor command by name instead of via keybind."
        },
        run = function()
            Core.Prompt.run("Run command: ", "", function(input)
                if not input or input:match("^%s*$") then return end

                local cmd = Core.Commands.get(input)
                if cmd then
                    if Core.Hooks.run_boolean("command::before-execute", input, cmd) then cmd.run() end
                else
                    Cini:set_status_message("Unknown command: " .. input, "error_message", 3000, false)
                end
            end)
        end
    })

    Core.Commands.register("global.undo", {
        metadata = {
            modifies = true,
            synopsis = "Undo",
            description = "Undoes the last text modification transaction."
        },
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
        metadata = {
            modifies = true,
            synopsis = "Redo",
            description = "Redoes the last undone text modification transaction."
        },
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
        metadata = {
            synopsis = "Close split",
            description = "Closes the current viewport split. If it is the last split, prompts to quit the editor."
        },
        run = function() if Cini.workspace:close_split() then Core.Quit.safe_quit() end end
    })

    Core.Commands.register("global.scroll_left", {
        metadata = { synopsis = "Scroll left", description = "Scrolls the viewport to the left." },
        run = function() Cini.workspace.viewport:scroll_left(1) end
    })
    Core.Commands.register("global.scroll_down", {
        metadata = { synopsis = "Scroll down", description = "Scrolls the viewport down." },
        run = function() Cini.workspace.viewport:scroll_down(1) end
    })
    Core.Commands.register("global.scroll_up", {
        metadata = { synopsis = "Scroll up", description = "Scrolls the viewport up." },
        run = function() Cini.workspace.viewport:scroll_up(1) end
    })
    Core.Commands.register("global.scroll_right", {
        metadata = { synopsis = "Scroll right", description = "Scrolls the viewport to the right." },
        run = function() Cini.workspace.viewport:scroll_right(1) end
    })

    Core.Commands.register("global.scroll_page_left", {
        metadata = { synopsis = "Scroll page left", description = "Scrolls the viewport to the left by its width." },
        run = function() Cini.workspace.viewport:scroll_left(Cini.workspace.viewport.width) end
    })
    Core.Commands.register("global.scroll_page_down", {
        metadata = { synopsis = "Scroll page down", description = "Scrolls the viewport down by its height." },
        run = function() Cini.workspace.viewport:scroll_down(Cini.workspace.viewport.height) end
    })
    Core.Commands.register("global.scroll_page_up", {
        metadata = { synopsis = "Scroll page up", description = "Scrolls the viewport up by its height." },
        run = function() Cini.workspace.viewport:scroll_up(Cini.workspace.viewport.height) end
    })
    Core.Commands.register("global.scroll_page_right", {
        metadata = { synopsis = "Scroll page right", description = "Scrolls the viewport to the right by its width." },
        run = function() Cini.workspace.viewport:scroll_right(Cini.workspace.viewport.width) end
    })

    Core.Commands.register("global.toggle_gutter", {
        metadata = {
            synopsis = "Toggle gutter",
            description = "Toggles the visibility of the line number gutter on the side."
        },
        run = function()
            local viewport = Cini.workspace.viewport

            viewport.view.gutter = not viewport.view.gutter
            viewport:adjust()
        end
    })
    Core.Commands.register("global.toggle_mode_line", {
        metadata = {
            synopsis = "Toggle mode line",
            description = "Toggles the visibility of the mode line at the bottom of the viewport."
        },
        run = function()
            local viewport = Cini.workspace.viewport

            viewport.view.mode_line = not viewport.view.mode_line
            viewport:adjust()
        end
    })

    Core.Commands.register("global.split_vertical", {
        metadata = { synopsis = "Split vertically", description = "Splits the current viewport vertically." },
        run = function() Cini.workspace:split_vertical(0.5) end
    })
    Core.Commands.register("global.split_horizontal", {
        metadata = { synopsis = "Split horizontally", description = "Splits the current viewport horizontally." },
        run = function() Cini.workspace:split_horizontal(0.5) end
    })
    Core.Commands.register("global.resize_split_inc", {
        metadata = { synopsis = "Increase split size", description = "Increases the width/height of the current split." },
        run = function() Cini.workspace:resize_split(0.05) end
    })
    Core.Commands.register("global.resize_split_dec", {
        metadata = { synopsis = "Decrease split size", description = "Decreases the width/height of the current split." },
        run = function() Cini.workspace:resize_split(-0.05) end
    })

    Core.Commands.register("global.navigate_split_left", {
        metadata = { synopsis = "Focus left split", description = "Moves focus to the split on the left." },
        run = function() Cini.workspace:navigate_split(Core.Direction.Left) end
    })
    Core.Commands.register("global.navigate_split_down", {
        metadata = { synopsis = "Focus split below", description = "Moves focus to the split below." },
        run = function() Cini.workspace:navigate_split(Core.Direction.Down) end
    })
    Core.Commands.register("global.navigate_split_up", {
        metadata = { synopsis = "Focus split above", description = "Moves focus to the split above." },
        run = function() Cini.workspace:navigate_split(Core.Direction.Up) end
    })
    Core.Commands.register("global.navigate_split_right", {
        metadata = { synopsis = "Focus right split", description = "Moves focus to the split on the right." },
        run = function() Cini.workspace:navigate_split(Core.Direction.Right) end
    })

    Core.Commands.register("global.delete_char", {
        metadata = {
            modifies = true,
            synopsis = "Delete character",
            description = "Deletes the character directly under the cursor."
        },
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
        metadata = {
            modifies = true,
            synopsis = "Delete line",
            description = "Deletes the entire current line."
        },
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
        metadata = {
            synopsis = "Yank line",
            description = "Copies the entire current line to the system clipboard."
        },
        run = function()
            local view = Cini.workspace.viewport.view
            local start = view.doc:line_begin_byte(view.cur.row)
            local stop = view.doc:line_end_byte(view.cur.row)

            if start ~= stop then Core.Clipboard.set_system_clipboard(view.doc:slice(start, stop)) end
        end
    })

    Core.Commands.register("global.paste", {
        metadata = {
            modifies = true,
            synopsis = "Paste",
            description = "Inserts the contents of the system clipboard at the cursor position."
        },
        run = function()
            local view = Cini.workspace.viewport.view

            view.doc:begin_transaction(view.cur:point(view))
            view.doc:insert(view.cur:point(view), Core.Clipboard.get_system_clipboard())
            view.doc:end_transaction(view.cur:point(view))
        end
    })

    Core.Commands.register("global.replace_char", {
        metadata = {
            modifies = true,
            synopsis = "Replace character",
            description = "Replaces the character immediately under the cursor with the next typed key."
        },
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

    Core.Commands.register("global.jump", {
        metadata = {
            synopsis = "Jump to line",
            description = "Prompts for a line number and jumps the cursor to the beginning of that line."
        },
        run = function()
            Core.Prompt.run("Jump to line: ", "", function(input)
                if not input or input:match("^%s*$") then return end

                local num = tonumber(input)
                if not num then
                    Cini:set_status_message("Invalid line number.", "error_message", 3000, false)
                    return
                end

                local view = Cini.workspace.viewport.view
                local doc = view.doc
                local curr_row = view.cur.row

                local max_row = doc:position_from_byte(doc.size).row

                local target_row = 0

                local first_char = input:sub(1, 1)
                if first_char == "+" or first_char == "-" then
                    -- Relative jump.
                    target_row = curr_row + num
                else
                    -- Absolute jump.
                    target_row = num - 1
                end

                target_row = math.max(0, math.min(target_row, max_row))

                view:move_cursor(function(c, v, _) c:move_to(v, doc:line_begin_byte(target_row)) end, 0)
                Cini.workspace.viewport:adjust()
            end)
        end
    })

    Core.Commands.register("global.new_document", {
        metadata = {
            synopsis = "New document",
            description = "Creates and opens a new scratchpad document."
        },
        run = function()
            Cini.workspace.viewport:change_document_view(Cini:create_document_view(Cini:create_document(nil)))
        end
    })
    Core.Commands.register("global.open_document", {
        metadata = {
            synopsis = "Open document",
            description = "Prompts for a file path to open in a new buffer."
        },
        run = function()
            local doc = Cini.workspace.viewport.view.doc
            local dir = Cini.pwd

            if doc and doc.path then
                local tmp = doc.path:match("^(.*[/\\])")
                if tmp ~= "" then dir = tmp end
            end

            Core.Prompt.run("Open: ", dir, function(input)
                Cini.workspace.viewport:change_document_view(
                    Cini:create_document_view(Cini:create_document(input ~= "" and input or nil)))
            end)
        end
    })
    Core.Commands.register("global.save_document", {
        metadata = {
            modifies = true,
            synopsis = "Save document",
            description = "Saves the current document. If it has no path, prompts for one."
        },
        run = function()
            local doc = Cini.workspace.viewport.view.doc
            Core.Prompt.run("Save: ", doc.path or Cini.pwd, function(input)
                if input ~= "" then doc:save(input) else doc:save(nil) end
                Cini:set_status_message("Saved file", "info_message", 3000, false)
            end)
        end
    })

    Core.Commands.register("global.health", {
        metadata = {
            synopsis = "Editor health",
            description = "Runs the garbage collector and displays debug statistics and memory usage."
        },
        run = function()
            collectgarbage()

            local stats = Cini:debug_stats()
            local msg = ("[Health] Docs: %d (%d tracked) | Views: %d (%d tracked) | Viewports: %d | Processes: %d (%d tracked)")
                :format(
                    stats.document_instances, stats.documents, stats.document_view_instances, stats.document_views,
                    stats.viewport_instances, stats.process_instances, stats.processes)

            Cini:set_status_message(msg, "info_message", 0, false)
        end
    })

    -- Keybinds.
    Core.Keybinds.bind("global", "<M-p>", "global.command_palette")

    Core.Keybinds.bind("global", "u", "global.undo")
    Core.Keybinds.bind("global", "U", "global.redo")

    Core.Keybinds.bind("global", "<C-q>", "global.close_split")

    Core.Keybinds.bind("global", "<S-h>", "global.scroll_left")
    Core.Keybinds.bind("global", "<S-j>", "global.scroll_down")
    Core.Keybinds.bind("global", "<S-k>", "global.scroll_up")
    Core.Keybinds.bind("global", "<S-l>", "global.scroll_right")

    Core.Keybinds.bind("global", "<S-Left>", "global.scroll_page_left")
    Core.Keybinds.bind("global", "<S-Down>", "global.scroll_page_down")
    Core.Keybinds.bind("global", "<S-Up>", "global.scroll_page_up")
    Core.Keybinds.bind("global", "<S-Right>", "global.scroll_page_right")

    Core.Keybinds.bind("global", "<C-w> g", "global.toggle_gutter")
    Core.Keybinds.bind("global", "<C-w> m", "global.toggle_mode_line")

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

    Core.Keybinds.bind("global", "r <CatchAll>", "global.replace_char")

    Core.Keybinds.bind("global", "<C-j>", "global.jump")

    Core.Keybinds.bind("global", "<C-n>", "global.new_document")
    Core.Keybinds.bind("global", "<C-o>", "global.open_document")
    Core.Keybinds.bind("global", "<C-s>", "global.save_document")

    Core.Keybinds.bind("global", "<S-Esc>", "global.health")
end

function Global.init()
    -- Motions.
    Core.Motions.register_motion("left", {
        sequences = { "h", "<Left>" },
        metadata = { synopsis = "Move left", description = "Moves the cursor left by characters." },
        run = function(cur, view, n) cur:left(view, n) end
    })
    Core.Motions.register_motion("down", {
        sequences = { "j", "<Down>" },
        metadata = { synopsis = "Move down", description = "Moves the cursor down by lines." },
        run = function(cur, view, n) cur:down(view, n) end
    })
    Core.Motions.register_motion("up", {
        sequences = { "k", "<Up>" },
        metadata = { synopsis = "Move up", description = "Moves the cursor up by lines." },
        run = function(cur, view, n) cur:up(view, n) end
    })
    Core.Motions.register_motion("right", {
        sequences = { "l", "<Right>" },
        metadata = { synopsis = "Move right", description = "Moves the cursor right by characters." },
        run = function(cur, view, n) cur:right(view, n) end
    })
    Core.Motions.register_motion("beginning_of_line", {
        sequences = { "<" },
        metadata = {
            synopsis = "Beginning of line",
            description = "Jumps to the first character of the current line."
        },
        run = function(cur, view, _) cur:_jump_to_beginning_of_line(view) end
    })
    Core.Motions.register_motion("end_of_line", {
        sequences = { ">" },
        metadata = { synopsis = "End of line", description = "Jumps to the last character of the current line." },
        run = function(cur, view, _) cur:_jump_to_end_of_line(view) end
    })
    Core.Motions.register_motion("beginning_of_file", {
        sequences = { "<S-g>" },
        metadata = { synopsis = "Beginning of file", description = "Jumps to the absolute beginning of the document." },
        run = function(cur, view, _) cur:_jump_to_beginning_of_file(view) end
    })
    Core.Motions.register_motion("end_of_file", {
        sequences = { "g" },
        metadata = { synopsis = "End of file", description = "Jumps to the absolute end of the document." },
        run = function(cur, view, _) cur:_jump_to_end_of_file(view) end
    })
    Core.Motions.register_motion("next_word", {
        sequences = { "w" },
        metadata = { synopsis = "Next word start", description = "Jumps to the beginning of the next word." },
        run = function(cur, view, n) cur:_next_word(view, n) end
    })
    Core.Motions.register_motion("next_word_end", {
        sequences = { "<S-w>" },
        metadata = { synopsis = "Next word end", description = "Jumps to the end of the next word." },
        run = function(cur, view, n) cur:_next_word_end(view, n) end
    })
    Core.Motions.register_motion("prev_word", {
        sequences = { "b" },
        metadata = { synopsis = "Previous word start", description = "Jumps to the beginning of the previous word." },
        run = function(cur, view, n) cur:_prev_word(view, n) end
    })
    Core.Motions.register_motion("prev_word_end", {
        sequences = { "<S-b>" },
        metadata = { synopsis = "Previous word end", description = "Jumps to the end of the previous word." },
        run = function(cur, view, n) cur:_prev_word_end(view, n) end
    })
    Core.Motions.register_motion("next_whitespace", {
        sequences = { "s" },
        metadata = { synopsis = "Next whitespace", description = "Jumps forward to the next whitespace character." },
        run = function(cur, view, n) cur:_next_whitespace(view, n) end
    })
    Core.Motions.register_motion("prev_whitespace", {
        sequences = { "<S-s>" },
        metadata = {
            synopsis = "Previous whitespace",
            description = "Jumps backward to the previous whitespace character."
        },
        run = function(cur, view, n) cur:_prev_whitespace(view, n) end
    })
    Core.Motions.register_motion("next_empty_line", {
        sequences = { "}" },
        metadata = {
            synopsis = "Next paragraph",
            description = "Jumps forward to the next empty line (paragraph boundary)."
        },
        run = function(cur, view, n) cur:_next_empty_line(view, n) end
    })
    Core.Motions.register_motion("prev_empty_line", {
        sequences = { "{" },
        metadata = {
            synopsis = "Previous paragraph",
            description = "Jumps backward to the previous empty line (paragraph boundary)."
        },
        run = function(cur, view, n) cur:_prev_empty_line(view, n) end
    })
    Core.Motions.register_motion("opposite", {
        sequences = { "." },
        metadata = {
            synopsis = "Matching pair",
            description = "Jumps to the matching opposite bracket, brace, or parenthesis."
        },
        run = function(cur, view, _) cur:_jump_to_matching_opposite(view) end
    })
end

return Global
