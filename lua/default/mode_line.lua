local ModeLine = {}

function ModeLine.init()
    Core.Hooks.add("viewport::created", function(viewport)
        viewport:set_mode_line(ModeLine.mode_line)
    end)
end

function ModeLine.mode_line(viewport)
    local doc = viewport.doc
    local cursor = viewport.cursor
    local ret = {}

    local major_mode = Core.Modes.get_major_mode(doc)
    if major_mode then
        table.insert(ret, { text = " " .. string.upper(major_mode.name) })
    else
        table.insert(ret, { text = " [No Mode]" })
    end

    table.insert(ret, {
        text = string.format(" [%s | %dB]", ((doc.path or ""):match("([^/]+)$") or "No Path"), doc.size)
    })

    if Core.Modes.has_minor_mode(doc, "insert") then
        table.insert(ret, { text = " [INS]" })
    end

    table.insert(ret, { spacer = true })

    table.insert(ret, { text = string.format(" %d:%d ", cursor.row + 1, cursor.col + 1), })

    -- Return a list of table segments.
    return ret
end

return ModeLine
