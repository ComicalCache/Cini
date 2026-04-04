local Selection = {}

--- @enum Selection.Kind
Selection.Kind = {
    Char = 0,
    Line = 1,
}

--- @type boolean
Selection.active = false
--- @type integer
Selection.anchor = 0
--- @type integer
Selection.anchor_row = 0
--- @type Selection.Kind
Selection.kind = Selection.Kind.Char

function Selection.init()
    Core.Modes.register_mode({
        name = "selection",
        cursor_style = Core.CursorStyle.SteadyBlock
    })

    Core.Hooks.add("cursor::after-move", 4, function(doc, _)
        if not Selection.active or not Core.Modes.has_minor_mode(doc, "selection") then return end

        local vp = Cini.workspace:find_viewport(function(v) return v.doc == doc end)
        if vp then Selection.update(vp) end
    end)

    Core.Commands.register("global.start_char_selection", {
        metadata = { modifies = false },
        run = function() Selection.start(Selection.Kind.Char) end
    })
    Core.Commands.register("global.start_line_selection", {
        metadata = { modifies = false },
        run = function() Selection.start(Selection.Kind.Line) end
    })
    Core.Keybinds.bind("global", "v", "global.start_char_selection")
    Core.Keybinds.bind("global", "V", "global.start_line_selection")

    Core.Commands.register("selection.cancel", {
        metadata = { modifies = false },
        run = function() Selection.stop() end
    })
    Core.Commands.register("selection.delete", {
        metadata = { modifies = true },
        run = function()
            local viewport = Cini.workspace.viewport
            local start, stop = Selection.get_range(viewport)
            Selection.stop()

            viewport.doc:begin_transaction(viewport.cursor:point(viewport.doc))
            viewport.doc:remove(start, stop)
            viewport.cursor:move_to(viewport.doc, start)
            viewport.doc:end_transaction(viewport.cursor:point(viewport.doc))
        end
    })
    Core.Commands.register("selection.change", {
        metadata = { modifies = true },
        run = function()
            local viewport = Cini.workspace.viewport
            local start, stop = Selection.get_range(viewport)
            Selection.stop()

            viewport.doc:begin_transaction(viewport.cursor:point(viewport.doc))
            viewport.doc:remove(start, stop)
            viewport.cursor:move_to(viewport.doc, start)

            Core.Modes.add_minor_mode(viewport.doc, "insert")

            -- The transaction isn't ended since we are in insert mode afterwards.
        end
    })
    Core.Commands.register("selection.yank", {
        metadata = { modifies = false },
        run = function()
            local viewport = Cini.workspace.viewport
            local start, stop = Selection.get_range(viewport)

            if start ~= stop then
                Core.Util.set_system_clipboard(viewport.doc:slice(start, stop))
            end

            Selection.stop()
        end
    })

    Core.Keybinds.bind("selection", "<Esc>", "selection.cancel")
    Core.Keybinds.bind("selection", "d", "selection.delete")
    Core.Keybinds.bind("selection", "c", "selection.change")
    Core.Keybinds.bind("selection", "y", "selection.yank")

    Core.Selection = Selection
end

--- Starts a selection
--- @param kind Selection.Kind kind
function Selection.start(kind)
    local viewport = Cini.workspace.viewport
    local doc = viewport.doc

    Selection.active = true
    Selection.kind = kind
    Selection.anchor = viewport.cursor:point(doc)
    Selection.anchor_row = viewport.cursor.row

    Core.Modes.add_minor_mode(doc, "selection")
    Selection.update(viewport)
end

--- Stops a selection
function Selection.stop()
    if not Selection.active then return end
    Selection.active = false

    local viewport = Cini.workspace.viewport
    local doc = viewport.doc

    Core.Modes.remove_minor_mode(doc, "selection")
    doc:clear_text_properties("selection")
end

function Selection.get_range(viewport)
    local doc = viewport.doc
    local start = 0
    local stop = 0

    if Selection.kind == Selection.Kind.Char then
        local current = viewport.cursor:point(doc)

        start = Selection.anchor
        stop = current
        if start > stop then start, stop = stop, start end
    else
        local p1 = Selection.anchor_row
        local p2 = viewport.cursor.row
        if p1 > p2 then p1, p2 = p2, p1 end

        start = doc:line_begin_byte(p1)
        stop = doc:line_end_byte(p2)
    end

    return start, stop
end

function Selection.update(viewport)
    local doc = viewport.doc

    doc:clear_text_properties("selection")

    local start, stop = Selection.get_range(viewport)
    if start ~= stop then doc:add_text_property(start, stop, "selection", "selection") end
end

return Selection
