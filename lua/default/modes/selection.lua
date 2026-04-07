local Selection = {}

--- @enum Selection.Kind
Selection.Kind = {
    Char = 0,
    Line = 1,
}

--- @class Selection.State
--- @field active boolean Is the selection currently active?
--- @field kind Selection.Kind The type of selection (Char or Line).
--- @field anchor integer The byte offset of the anchor point.
--- @field anchor_row integer The row index of the anchor point.
local State = {}

function Selection.init()
    Core.Modes.register_mode({
        name = "selection",
        cursor_style = Core.CursorStyle.SteadyBlock
    })

    Core.Hooks.add("cursor::after-move", 4, function(view, _)
        --- @cast view Core.DocumentView

        --- @type Selection.State?
        local state = view.properties["selection"]
        if not state or not state.active or not Core.Modes.has_minor_mode(view, "selection") then return end

        Selection.update(view)
    end)

    Core.Hooks.add("document::after-insert", 5, function(doc, start, len)
        --- @cast doc Core.Document
        --- @cast start integer
        --- @cast len integer

        for _, view in ipairs(doc:views()) do
            --- @type Selection.State?
            local state = view.properties["selection"]
            if state and state.active then
                if state.anchor > start then state.anchor = state.anchor + len end
                state.anchor_row = doc:position_from_byte(state.anchor).row

                Selection.update(view)
            end
        end
    end)
    Core.Hooks.add("document::after-remove", 5, function(doc, start, len)
        --- @cast doc Core.Document
        --- @cast start integer
        --- @cast len integer

        for _, view in ipairs(doc:views()) do
            --- @type Selection.State?
            local state = view.properties["selection"]
            if state and state.active then
                if state.anchor > start then
                    if state.anchor <= start + len then -- Anchor was inside the deleted range.
                        state.anchor = start
                    else                                -- Anchor was after the deleted range.
                        state.anchor = state.anchor - len
                    end
                end
                state.anchor_row = doc:position_from_byte(state.anchor).row

                Selection.update(view)
            end
        end
    end)
    Core.Hooks.add("document::after-clear", 5, function(doc)
        --- @cast doc Core.Document

        for _, view in ipairs(doc:views()) do Selection.stop(view) end
    end)

    Core.Commands.register("global.start_char_selection", {
        metadata = { modifies = false },
        run = function() Selection.start(Cini.workspace.viewport.view, Selection.Kind.Char) end
    })
    Core.Commands.register("global.start_line_selection", {
        metadata = { modifies = false },
        run = function() Selection.start(Cini.workspace.viewport.view, Selection.Kind.Line) end
    })
    Core.Keybinds.bind("global", "v", "global.start_char_selection")
    Core.Keybinds.bind("global", "V", "global.start_line_selection")

    Core.Commands.register("selection.cancel", {
        metadata = { modifies = false },
        run = function() Selection.stop(Cini.workspace.viewport.view) end
    })
    Core.Commands.register("selection.delete", {
        metadata = { modifies = true },
        run = function()
            local view = Cini.workspace.viewport.view
            local start, stop = Selection.get_range(view)
            Selection.stop(view)

            view.doc:begin_transaction(view.cur:point(view))
            view.doc:remove(start, stop)
            view:move_cursor(function(c, v, _) c:move_to(v, start) end, 0)
            view.doc:end_transaction(view.cur:point(view))
        end
    })
    Core.Commands.register("selection.change", {
        metadata = { modifies = true },
        run = function()
            local view = Cini.workspace.viewport.view
            local start, stop = Selection.get_range(view)
            Selection.stop(view)

            view.doc:begin_transaction(view.cur:point(view))
            view.doc:remove(start, stop)
            view:move_cursor(function(c, v, _) c:move_to(v, start) end, 0)

            Core.Modes.add_minor_mode(view, "insert")

            -- The transaction isn't ended since we are in insert mode afterwards.
        end
    })
    Core.Commands.register("selection.yank", {
        metadata = { modifies = false },
        run = function()
            local view = Cini.workspace.viewport.view
            local start, stop = Selection.get_range(view)

            if start ~= stop then
                Core.Util.set_system_clipboard(view.doc:slice(start, stop))
            end

            Selection.stop(view)
        end
    })

    Core.Keybinds.bind("selection", "<Esc>", "selection.cancel")
    Core.Keybinds.bind("selection", "d", "selection.delete")
    Core.Keybinds.bind("selection", "c", "selection.change")
    Core.Keybinds.bind("selection", "y", "selection.yank")

    Core.Selection = Selection
end

--- @param view Core.DocumentView
--- @param kind Selection.Kind
function Selection.start(view, kind)
    --- @type Selection.State
    local state = {
        active = true,
        kind = kind,
        anchor = view.cur:point(view),
        anchor_row = view.cur.row
    }

    view.properties["selection"] = state

    Core.Modes.add_minor_mode(view, "selection")
    Selection.update(view)
end

--- @param view Core.DocumentView
function Selection.stop(view)
    --- @type Selection.State?
    local state = view.properties["selection"]
    if not state or not state.active then return end

    view.properties["selection"] = nil

    Core.Modes.remove_minor_mode(view, "selection")
    view:clear_view_properties("selection")
end

--- @param view Core.DocumentView
--- @return integer start, integer stop
function Selection.get_range(view)
    local doc = view.doc

    --- @type Selection.State?
    local state = view.properties["selection"]
    if not state then return 0, 0 end

    local start = 0
    local stop = 0

    if state.kind == Selection.Kind.Char then
        local current = view.cur:point(view)
        start = state.anchor
        stop = current

        if start > stop then start, stop = stop, start end
    else
        local p1 = state.anchor_row
        local p2 = view.cur.row

        if p1 > p2 then p1, p2 = p2, p1 end

        start = doc:line_begin_byte(p1)
        stop = doc:line_end_byte(p2)
    end

    return start, stop
end

--- Updates highlighting of the selection.
--- @param view Core.DocumentView
function Selection.update(view)
    view:clear_view_properties("selection")

    local start, stop = Selection.get_range(view)
    if start ~= stop then
        view:add_view_property(start, stop, "selection", "selection")
    end
end

return Selection
