local M = {}

function M.post_init()
    State.editor.viewport:set_mode_line(M.mode_line)
end

function M.mode_line(viewport)
    local Mode = require("core.internals.mode")

    local doc = viewport.doc
    local cursor = viewport.cursor
    local ret = {}

    local major_mode = Mode.get_major_mode(doc)
    if major_mode then
        table.insert(ret, {
            text = " " .. string.upper(major_mode.name),
            face = "mode_line"
        })
    else
        table.insert(ret, {
            text = " [No Mode]",
            face = "mode_line"
        })
    end

    table.insert(ret, {
        text = string.format(" [%s | %dB]", (doc.path:match("([^/]+)$") or "No Path"), doc.size),
        face = "mode_line"
    })

    table.insert(ret, {
        spacer = true,
        face = "mode_line"
    })

    table.insert(ret, {
        text = string.format(" %d:%d ", cursor.row + 1, cursor.col + 1),
        face = "mode_line"
    })

    -- Return a list of table segments.
    return ret
end

return M
