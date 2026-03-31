local MiniBuffer = {}

function MiniBuffer.init()
    Core.Modes.register_mode(Core.Mode.new({
        name = "error_message",
        faces = {
            default = Core.Face({ fg = Core.Rgb(224, 108, 117), bg = Core.Rgb(41, 44, 51) }),
        }
    }))
    Core.Modes.register_mode(Core.Mode.new({
        name = "info_message",
        faces = {
            default = Core.Face({ fg = Core.Rgb(97, 175, 239), bg = Core.Rgb(41, 44, 51) }),
        }
    }))

    Core.Hooks.add("mini_buffer::created", 1, function()
        Core.Modes.set_major_mode(Cini.workspace.mini_buffer.doc, "mini_buffer")
    end)
    Core.Hooks.add("cursor::after-move", 1, function(_, _)
        if not Cini.workspace.is_mini_buffer then
            Cini:clear_status_message()
        end
    end)

    Core.Keybinds.bind("mini_buffer", "<Esc>", function()
        Cini.workspace:exit_mini_buffer()
    end)
    Core.Keybinds.bind("mini_buffer", "<Left>", function()
        Cini.workspace.mini_buffer:move_cursor(Core.Cursor.left, 1)
    end)
    Core.Keybinds.bind("mini_buffer", "<M-Left>", function()
        Cini.workspace.mini_buffer:move_cursor(Core.Cursor._prev_word, 1)
    end)
    Core.Keybinds.bind("mini_buffer", "<Right>", function()
        Cini.workspace.mini_buffer:move_cursor(Core.Cursor.right, 1)
    end)
    Core.Keybinds.bind("mini_buffer", "<M-Right>", function()
        Cini.workspace.mini_buffer:move_cursor(Core.Cursor._next_word, 1)
    end)
    Core.Keybinds.bind("mini_buffer", "<Down>", function()
        Cini.workspace.mini_buffer:move_cursor(Core.Cursor.down, 1)
    end)
    Core.Keybinds.bind("mini_buffer", "<Up>", function()
        Cini.workspace.mini_buffer:move_cursor(Core.Cursor.up, 1)
    end)

    Core.Keybinds.bind("mini_buffer", "<Space>", function()
        local viewport = Cini.workspace.mini_buffer
        local doc = viewport.doc

        doc:insert(viewport.cursor:point(doc), " ")
        viewport:move_cursor(Core.Cursor.right, 1)
    end)
    Core.Keybinds.bind("mini_buffer", "<S-Enter>", function()
        local viewport = Cini.workspace.mini_buffer
        local doc = viewport.doc

        doc:insert(viewport.cursor:point(doc), "\n")
        viewport:move_cursor(Core.Cursor.down, 1)
        viewport:move_cursor(function(cur, d, _) cur:_jump_to_beginning_of_line(d) end, 1)
    end)
    Core.Keybinds.bind("mini_buffer", "<Tab>", function()
        local viewport = Cini.workspace.mini_buffer
        local doc = viewport.doc

        doc:insert(viewport.cursor:point(doc), "\t")
        viewport:move_cursor(Core.Cursor.right, 1)
    end)
    Core.Keybinds.bind("mini_buffer", "<Bspc>", function()
        local viewport = Cini.workspace.mini_buffer
        local doc = viewport.doc
        local point = viewport.cursor:point(doc)


        if point ~= 0 and viewport:move_cursor(Core.Cursor.left, 1) then
            doc:remove(viewport.cursor:point(doc), point)
        end
    end)
    Core.Keybinds.bind("mini_buffer", "<Del>", function()
        local viewport = Cini.workspace.mini_buffer
        local doc = viewport.doc
        local point = viewport.cursor:point(doc)

        if point ~= doc.size then
            doc:remove(point, point + Core.Utf8.len(doc:slice(point, point + 1)))
        end
    end)

    Core.Keybinds.bind("mini_buffer", "<CatchAll>", function(key_str)
        local viewport = Cini.workspace.mini_buffer
        local doc = viewport.doc

        doc:insert(viewport.cursor:point(doc), key_str)
        viewport:move_cursor(Core.Cursor.right, Core.Utf8.count(key_str))

        return true
    end)
end

return MiniBuffer
