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
    local minor_mode_override = Core.Modes.get_minor_mode_override(doc)
    if major_mode and minor_mode_override then
        table.insert(ret,
            { text = " [" .. string.upper(major_mode.name) .. " | " .. string.upper(minor_mode_override.name) .. "]" })
    elseif major_mode then
        table.insert(ret, { text = " [" .. string.upper(major_mode.name) .. "]" })
    elseif minor_mode_override then
        table.insert(ret, { text = " [" .. string.upper(minor_mode_override.name) .. "]" })
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

    return ret
end

return ModeLine
