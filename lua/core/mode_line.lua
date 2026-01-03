local M = {}

function M.setup()
    State.editor.active_viewport:set_mode_line(M.mode_line)
end

function M.mode_line(viewport)
    local doc = viewport.doc
    local cursor = viewport.cursor

    local major_mode_name = " [No Mode]"
    if doc.major_mode.name ~= "" then
        major_mode_name = " " .. string.upper(doc.major_mode.name)
    end

    -- Return a list of table segments.
    return {
        {
            text = major_mode_name,
            face = "mode_line_default"
        },
        {
            text = " " .. (doc.path or "[No Path]"),
            face = "mode_line_default"
        },
        {
            spacer = true,
            face = "mode_line_default"
        },
        {
            text = string.format(" %d:%d ", cursor.row + 1, cursor.col + 1),
            face = "mode_line_default"
        }
    }
end

return M
