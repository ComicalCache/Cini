local Indent = {}

function Indent.setup()
    -- Commands.
    Core.Commands.register("global.indent", {
        metadata = { modifies = true },
        run = function()
            local view = Cini.workspace.viewport.view
            local point = view.cur:point(view)

            view.doc:begin_transaction(point)
            Indent.indent(view, point, point)
            view.doc:end_transaction(view.cur:point(view))
        end
    })

    Core.Commands.register("global.unindent", {
        metadata = { modifies = true },
        run = function()
            local view = Cini.workspace.viewport.view
            local point = view.cur:point(view)

            view.doc:begin_transaction(point)
            Indent.unindent(view, point, point)
            view.doc:end_transaction(view.cur:point(view))
        end
    })

    -- Keybinds.
    Core.Keybinds.bind("global", "<Tab>", "global.indent")
    Core.Keybinds.bind("global", "<S-Tab>", "global.unindent")
end

function Indent.init() end

--- @param view Core.DocumentView
--- @param start integer
--- @param stop integer
function Indent.indent(view, start, stop)
    local doc = view.doc
    local start_pos = doc:position_from_byte(start)
    local stop_pos = doc:position_from_byte(stop)

    local start_row = start_pos.row
    local stop_row = stop_pos.row

    -- Don't indent lines if the cursor is at the beginning of it.
    if start ~= stop and stop == doc:line_begin_byte(stop_row) and stop_row > start_row then
        stop_row = stop_row - 1
    end

    local tab_width = view.properties["tab_width"] or 4
    local insert_str = string.rep(" ", tab_width)

    for row = stop_row, start_row, -1 do
        local line_start = doc:line_begin_byte(row)
        doc:insert(line_start, insert_str)
    end
end

--- @param view Core.DocumentView
--- @param start integer
--- @param stop integer
function Indent.unindent(view, start, stop)
    local doc = view.doc
    local start_pos = doc:position_from_byte(start)
    local stop_pos = doc:position_from_byte(stop)

    local start_row = start_pos.row
    local stop_row = stop_pos.row

    -- Don't indent lines if the cursor is at the beginning of it.
    if start ~= stop and stop == doc:line_begin_byte(stop_row) and stop_row > start_row then
        stop_row = stop_row - 1
    end

    local tab_width = view.properties["tab_width"] or 4

    for row = stop_row, start_row, -1 do
        local line_start = doc:line_begin_byte(row)
        local line_text = doc:slice(line_start, doc:line_end_byte(row))

        local ws_count = 0
        if line_text:sub(1, 1) == "\t" then
            ws_count = 1
        else
            for idx = 1, tab_width do
                if line_text:sub(idx, idx) == " " then
                    ws_count = ws_count + 1
                else
                    break
                end
            end
        end

        if ws_count > 0 then
            doc:remove(line_start, line_start + ws_count)
        end
    end
end

return Indent
