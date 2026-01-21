--- @class Core.Mode
--- @field name string The name of the mode.
--- @field keymap? table<string, function|table> Keybindings for this mode.
--- @field faces? table<string, Core.Face> Face definitions for this mode.
local Mode

local M = {}

-- Global mode registry.
--- @type table<string, Core.Mode>
M.modes = {}

--- @param name string
--- @param mode Core.Mode
function M.register_mode(name, mode)
    if not mode.name then
        mode.name = name
    end

    M.modes[name] = mode
end

--- @param mode string
--- @return Core.Mode?
function M.get_mode(mode)
    if M.modes[mode] then
        return M.modes[mode]
    end

    return nil
end

--- @param mode string
--- @return boolean
function M.has_mode(mode)
    return M.modes[mode] ~= nil
end

--- @param doc Core.Document
--- @return Core.Mode?
function M.get_major_mode(doc)
    return doc.properties["major_mode"]
end

--- @param doc Core.Document
--- @param name string
function M.set_major_mode(doc, name)
    local mode = M.get_mode(name)
    if not mode then
        -- TODO: log error.
    end

    doc.properties["major_mode"] = mode
end

--- @param doc Core.Document
--- @return Core.Mode[]
function M.get_minor_modes(doc)
    local modes = doc.properties["minor_modes"]
    if not modes or type(modes) ~= "table" then
        return {}
    end

    return modes
end

--- @param doc Core.Document
--- @param name string
function M.add_minor_mode(doc, name)
    if M.has_minor_mode(doc, name) then
        return
    end

    local mode = M.get_mode(name)
    if not mode then
        -- TODO: log error.
    end

    local modes = doc.properties["minor_modes"]
    if not modes or type(modes) ~= "table" then
        modes = {}
    end
    table.insert(modes, mode)

    doc.properties["minor_modes"] = modes
end

--- @param doc Core.Document
--- @param name string
function M.remove_minor_mode(doc, name)
    local modes = doc.properties["minor_modes"]
    if not modes or type(modes) ~= "table" then
        return
    end

    for idx = #modes, 1, -1 do
        if modes[idx].name == name then
            table.remove(modes, idx)
            break
        end
    end

    if #modes > 0 then
        doc.properties["minor_modes"] = modes
    else
        doc.properties["minor_modes"] = nil
    end
end

--- @param doc Core.Document
--- @param name string
--- @return boolean
function M.has_minor_mode(doc, name)
    local modes = M.get_minor_modes(doc)
    for _, mode in ipairs(modes) do
        if mode.name == name then
            return true
        end
    end

    return false
end

--- @param doc Core.Document
--- @return Core.Mode?
function M.get_minor_mode_override(doc)
    return doc.properties["minor_mode_override"]
end

--- @param doc Core.Document
--- @param name string
function M.set_minor_mode_override(doc, name)
    local mode = M.get_mode(name)
    if not mode then
        -- TODO: log error.
    end

    doc.properties["minor_mode_override"] = mode
end

--- @param doc Core.Document
function M.remove_minor_mode_override(doc)
    doc.properties["minor_mode_override"] = nil
end

return M
