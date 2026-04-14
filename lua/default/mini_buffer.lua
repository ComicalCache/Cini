local MiniBuffer = {}

function MiniBuffer.setup()
    -- Modes.
    local current_line_override = Core.Faces.get_face("default") or {}
    Core.Modes.register_mode({
        name = "mini_buffer",
        faces = { current_line = Core.Face({ bg = current_line_override.bg }) }
    })

    -- Hooks.
    Core.Hooks.add("mini_buffer::created", 10, function()
        Core.Modes.set_major_mode(Cini.workspace.mini_buffer.view.doc, "mini_buffer")
    end)
    Core.Hooks.add("cursor::after-move", 10, function(_, _)
        if not Cini.workspace.is_mini_buffer then Cini:clear_status_message() end
    end)

    -- Commands.
    Core.Commands.register("mini_buffer.exit",
        { metadata = {}, run = function() Cini.workspace:exit_mini_buffer() end })

    Core.Commands.register("mini_buffer.move_left", {
        metadata = {},
        run = function() Cini.workspace.mini_buffer.view:move_cursor(Core.Cursor.left, 1) end
    })
    Core.Commands.register("mini_buffer.move_prev_word", {
        metadata = {},
        run = function() Cini.workspace.mini_buffer.view:move_cursor(Core.Cursor._prev_word, 1) end
    })
    Core.Commands.register("mini_buffer.move_right", {
        metadata = {},
        run = function() Cini.workspace.mini_buffer.view:move_cursor(Core.Cursor.right, 1) end
    })
    Core.Commands.register("mini_buffer.move_next_word", {
        metadata = {},
        run = function() Cini.workspace.mini_buffer.view:move_cursor(Core.Cursor._next_word, 1) end
    })
    Core.Commands.register("mini_buffer.move_down", {
        metadata = {},
        run = function() Cini.workspace.mini_buffer.view:move_cursor(Core.Cursor.down, 1) end
    })
    Core.Commands.register("mini_buffer.move_up", {
        metadata = {},
        run = function() Cini.workspace.mini_buffer.view:move_cursor(Core.Cursor.up, 1) end
    })

    -- Modify text.
    Core.Commands.register("mini_buffer.space", {
        metadata = { modifies = true },
        run = function()
            local view = Cini.workspace.mini_buffer.view

            view.doc:insert(view.cur:point(view), " ")
            view:move_cursor(Core.Cursor.right, 1)
        end
    })
    Core.Commands.register("mini_buffer.enter", {
        metadata = { modifies = true },
        run = function()
            local view = Cini.workspace.mini_buffer.view

            view.doc:insert(view.cur:point(view), "\n")
            view:move_cursor(Core.Cursor.down, 1)
            view:move_cursor(function(c, v, _) c:_jump_to_beginning_of_line(v) end, 1)
        end
    })
    Core.Commands.register("mini_buffer.tab", {
        metadata = { modifies = true },
        run = function()
            local view = Cini.workspace.mini_buffer.view

            view.doc:insert(view.cur:point(view), "\t")
            view:move_cursor(Core.Cursor.right, 1)
        end
    })
    Core.Commands.register("mini_buffer.backspace", {
        metadata = { modifies = true },
        run = function()
            local view = Cini.workspace.mini_buffer.view
            local point = view.cur:point(view)

            if point ~= 0 and view:move_cursor(Core.Cursor.left, 1) then
                view.doc:remove(view.cur:point(view), point)
            end
        end
    })
    Core.Commands.register("mini_buffer.delete", {
        metadata = { modifies = true },
        run = function()
            local view = Cini.workspace.mini_buffer.view
            local point = view.cur:point(view)

            if point ~= view.doc.size then
                view.doc:remove(point, point + Core.Utf8.len(view.doc:slice(point, point + 1)))
            end
        end
    })

    -- Insert text.
    Core.Commands.register("mini_buffer.insert", {
        metadata = { modifies = true },
        run = function(key_str)
            local view = Cini.workspace.mini_buffer.view

            view.doc:insert(view.cur:point(view), key_str)
            view:move_cursor(Core.Cursor.right, Core.Utf8.count(key_str))

            return true
        end
    })

    -- Keybinds.
    Core.Keybinds.bind("mini_buffer", "<Esc>", "mini_buffer.exit")

    Core.Keybinds.bind("mini_buffer", "<Left>", "mini_buffer.move_left")
    Core.Keybinds.bind("mini_buffer", "<M-Left>", "mini_buffer.move_prev_word")
    Core.Keybinds.bind("mini_buffer", "<Right>", "mini_buffer.move_right")
    Core.Keybinds.bind("mini_buffer", "<M-Right>", "mini_buffer.move_next_word")
    Core.Keybinds.bind("mini_buffer", "<Down>", "mini_buffer.move_down")
    Core.Keybinds.bind("mini_buffer", "<Up>", "mini_buffer.move_up")

    Core.Keybinds.bind("mini_buffer", "<Space>", "mini_buffer.space")
    Core.Keybinds.bind("mini_buffer", "<S-Enter>", "mini_buffer.enter")
    Core.Keybinds.bind("mini_buffer", "<Tab>", "mini_buffer.tab")
    Core.Keybinds.bind("mini_buffer", "<Bspc>", "mini_buffer.backspace")
    Core.Keybinds.bind("mini_buffer", "<Del>", "mini_buffer.delete")

    Core.Keybinds.bind("mini_buffer", "<CatchAll>", "mini_buffer.insert")
end

function MiniBuffer.init() end

return MiniBuffer
