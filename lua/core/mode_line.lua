local M = {}

function M.setup()
    State.editor.active_viewport:set_mode_line(M.mode_line)
end

function M.mode_line(viewport)
    local doc = viewport.doc
    local cursor = viewport.cursor

    local ret = {}

    if doc.major_mode ~= nil and doc.major_mode.name ~= "" then
        table.insert(ret, {
            text = " " .. string.upper(doc.major_mode.name),
            face = "mode_line_default"
        })
    else
        table.insert(ret, {
            text = " [No Mode]",
            face = "mode_line_default"
        })
    end

    table.insert(ret, {
        text = string.format("[%s %dB]", (doc.path or "No Path"), doc.size),
        face = "mode_line_default"
    })

    table.insert(ret, {
        spacer = true,
        face = "mode_line_default"
    })

     if doc:has_minor_mode("insert") then
        table.insert(ret, {
            text = " [INS]",
            face = "mode_line_default"
        })
    end

    table.insert(ret, {
        text = string.format(" %d:%d ", cursor.row + 1, cursor.col + 1),
        face = "mode_line_default"
    })

    -- Return a list of table segments.
    return ret
end

return M
