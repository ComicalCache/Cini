local ModeLine = {}

function ModeLine.init()
    Core.Hooks.add("viewport::created", 1, function(viewport)
        viewport:set_mode_line(ModeLine.mode_line)
    end)
end

function ModeLine.mode_line(viewport)
    --- @cast viewport Core.Viewport

    local view = viewport.view

    local major_mode = Core.Modes.get_major_mode(view.doc)
    if major_mode and major_mode.mode_line then return major_mode.mode_line(viewport) end

    local ret = {}

    local minor_mode_override = Core.Modes.get_minor_mode_override(view)
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

    if Core.Modes.has_minor_mode(view, "insert") then
        table.insert(ret, { text = " " })
        table.insert(ret, { text = "[INS]", face = "selection" })
    elseif Core.Modes.has_minor_mode(view, "selection") then
        table.insert(ret, { text = " " })
        table.insert(ret, { text = "[SEL]", face = "selection" })
    else
        table.insert(ret, { text = " [VIS]" })
    end

    local name = view.doc.properties["name"] or ((view.doc.path or ""):match("([^/]+)$") or "Scratchpad")
    table.insert(ret, { text = (" [%s%s | %dB]"):format(name, (view.doc.modified and " *" or ""), view.doc.size) })

    local pending_keys = Core.Keybinds.pending_keys
    if pending_keys and #pending_keys > 0 then
        table.insert(ret, {
            text = " " .. table.concat(pending_keys, " "),
            face = Core.Face({ fg = Core.Rgb(97, 175, 239) })
        })
    end

    table.insert(ret, { spacer = true })

    table.insert(ret, { text = (" %d:%d "):format(view.cur.row + 1, view.cur.col + 1), })

    return ret
end

return ModeLine
