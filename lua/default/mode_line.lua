local ModeLine = {}

function ModeLine.init()
    Core.Hooks.add("viewport::created", 1, function(viewport)
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
            { text = " [" .. major_mode.name:upper() .. " | " .. minor_mode_override.name:upper() .. "]" })
    elseif major_mode then
        table.insert(ret, { text = " [" .. major_mode.name:upper() .. "]" })
    elseif minor_mode_override then
        table.insert(ret, { text = " [" .. minor_mode_override.name:upper() .. "]" })
    else
        table.insert(ret, { text = " [No Mode]" })
    end

    if Core.Modes.has_minor_mode(doc, "insert") then
        table.insert(ret, { text = " " })
        table.insert(ret,
            { text = "[INS]", face = Core.Face({ fg = Core.Rgb(41, 44, 51), bg = Core.Rgb(97, 175, 239) }) })
    else
        table.insert(ret, { text = " [VIS]" })
    end

    local name = doc.properties["name"] or ((doc.path or ""):match("([^/]+)$") or "Scratchpad")
    table.insert(ret, { text = (" [%s [%s] | %dB]"):format(name, (doc.modified and "*" or " "), doc.size) })

    local pending_keys = Core.Keybinds.pending_keys
    if pending_keys and #pending_keys > 0 then
        table.insert(ret, {
            text = " " .. table.concat(pending_keys, " "),
            face = Core.Face({ fg = Core.Rgb(97, 175, 239) })
        })
    end

    table.insert(ret, { spacer = true })

    table.insert(ret, { text = (" %d:%d "):format(cursor.row + 1, cursor.col + 1), })

    return ret
end

return ModeLine
