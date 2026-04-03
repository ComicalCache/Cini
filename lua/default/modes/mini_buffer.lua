local MiniBuffer = {}

function MiniBuffer.init()
    Core.Faces.register_face("error_message", Core.Face({ fg = Core.Rgb(224, 108, 117), bg = Core.Rgb(41, 44, 51) }))
    Core.Faces.register_face("info_message", Core.Face({ fg = Core.Rgb(97, 175, 239), bg = Core.Rgb(41, 44, 51) }))

    Core.Modes.register_mode({
        name = "error_message",
        faces = { default = "error_message" }
    })
    Core.Modes.register_mode({
        name = "info_message",
        faces = { default = "info_message" }
    })

    Core.Hooks.add("mini_buffer::created", 1, function()
        Core.Modes.set_major_mode(Cini.workspace.mini_buffer.doc, "mini_buffer")
    end)
    Core.Hooks.add("cursor::after-move", 1, function(_, _)
        if not Cini.workspace.is_mini_buffer then Cini:clear_status_message() end
    end)

    -- Exit.
    Core.Commands.register("mini_buffer.exit",
        { metadata = { modifies = false }, run = function() Cini.workspace:exit_mini_buffer() end })
    Core.Keybinds.bind("mini_buffer", "<Esc>", "mini_buffer.exit")

    -- Movement.
    Core.Commands.register("mini_buffer.move_left", {
        metadata = { modifies = false },
        run = function() Cini.workspace.mini_buffer:move_cursor(Core.Cursor.left, 1) end
    })
    Core.Commands.register("mini_buffer.move_prev_word", {
        metadata = { modifies = false },
        run = function() Cini.workspace.mini_buffer:move_cursor(Core.Cursor._prev_word, 1) end
    })
    Core.Commands.register("mini_buffer.move_right", {
        metadata = { modifies = false },
        run = function() Cini.workspace.mini_buffer:move_cursor(Core.Cursor.right, 1) end
    })
    Core.Commands.register("mini_buffer.move_next_word", {
        metadata = { modifies = false },
        run = function() Cini.workspace.mini_buffer:move_cursor(Core.Cursor._next_word, 1) end
    })
    Core.Commands.register("mini_buffer.move_down", {
        metadata = { modifies = false },
        run = function() Cini.workspace.mini_buffer:move_cursor(Core.Cursor.down, 1) end
    })
    Core.Commands.register("mini_buffer.move_up", {
        metadata = { modifies = false },
        run = function() Cini.workspace.mini_buffer:move_cursor(Core.Cursor.up, 1) end
    })
    Core.Keybinds.bind("mini_buffer", "<Left>", "mini_buffer.move_left")
    Core.Keybinds.bind("mini_buffer", "<M-Left>", "mini_buffer.move_prev_word")
    Core.Keybinds.bind("mini_buffer", "<Right>", "mini_buffer.move_right")
    Core.Keybinds.bind("mini_buffer", "<M-Right>", "mini_buffer.move_next_word")
    Core.Keybinds.bind("mini_buffer", "<Down>", "mini_buffer.move_down")
    Core.Keybinds.bind("mini_buffer", "<Up>", "mini_buffer.move_up")

    -- Modify text.
    Core.Commands.register("mini_buffer.space", {
        metadata = { modifies = true },
        run = function()
            local viewport = Cini.workspace.mini_buffer

            viewport.doc:insert(viewport.cursor:point(viewport.doc), " ")
            viewport:move_cursor(Core.Cursor.right, 1)
        end
    })
    Core.Commands.register("mini_buffer.enter", {
        metadata = { modifies = true },
        run = function()
            local viewport = Cini.workspace.mini_buffer

            viewport.doc:insert(viewport.cursor:point(viewport.doc), "\n")
            viewport:move_cursor(Core.Cursor.down, 1)
            viewport:move_cursor(function(cur, d, _) cur:_jump_to_beginning_of_line(d) end, 1)
        end
    })
    Core.Commands.register("mini_buffer.tab", {
        metadata = { modifies = true },
        run = function()
            local viewport = Cini.workspace.mini_buffer

            viewport.doc:insert(viewport.cursor:point(viewport.doc), "\t")
            viewport:move_cursor(Core.Cursor.right, 1)
        end
    })
    Core.Commands.register("mini_buffer.backspace", {
        metadata = { modifies = true },
        run = function()
            local viewport = Cini.workspace.mini_buffer
            local doc = viewport.doc
            local point = viewport.cursor:point(doc)

            if point ~= 0 and viewport:move_cursor(Core.Cursor.left, 1) then
                doc:remove(viewport.cursor:point(doc), point)
            end
        end
    })
    Core.Commands.register("mini_buffer.delete", {
        metadata = { modifies = true },
        run = function()
            local viewport = Cini.workspace.mini_buffer
            local doc = viewport.doc
            local point = viewport.cursor:point(doc)

            if point ~= doc.size then doc:remove(point, point + Core.Utf8.len(doc:slice(point, point + 1))) end
        end
    })
    Core.Keybinds.bind("mini_buffer", "<Space>", "mini_buffer.space")
    Core.Keybinds.bind("mini_buffer", "<S-Enter>", "mini_buffer.enter")
    Core.Keybinds.bind("mini_buffer", "<Tab>", "mini_buffer.tab")
    Core.Keybinds.bind("mini_buffer", "<Bspc>", "mini_buffer.backspace")
    Core.Keybinds.bind("mini_buffer", "<Del>", "mini_buffer.delete")

    -- Insert text.
    Core.Commands.register("mini_buffer.insert", {
        metadata = { modifies = true },
        run = function(key_str)
            local viewport = Cini.workspace.mini_buffer
            local doc = viewport.doc

            doc:insert(viewport.cursor:point(doc), key_str)
            viewport:move_cursor(Core.Cursor.right, Core.Utf8.count(key_str))

            return true
        end
    })
    Core.Keybinds.bind("mini_buffer", "<CatchAll>", "mini_buffer.insert")
end

return MiniBuffer
