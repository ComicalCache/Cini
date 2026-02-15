--- @class Core.Modes
local Modes = {}

--- Global mode registry.
--- @type table<string, Core.Mode>
Modes.modes = {}

function Modes.init()
    Core.Modes = Modes
end

--- Registers a named mode.
--- @param name string
--- @param mode Core.Mode
function Modes.register_mode(name, mode)
    mode.name = name
    Modes.modes[name] = mode
end

--- Retrieves a mode by name.
--- @param mode string
--- @return Core.Mode?
function Modes.get_mode(mode)
    return Modes.modes[mode]
end

--- Checks if a mode is registered.
--- @param mode string
--- @return boolean
function Modes.has_mode(mode)
    return Modes.modes[mode] ~= nil
end

--- Gets the Major Mode of a Document.
--- @param doc Core.Document
--- @return Core.Mode?
function Modes.get_major_mode(doc)
    return Modes.get_mode(doc.properties["major_mode"])
end

--- Sets the Major Mode of a Document.
--- @param doc Core.Document
--- @param mode string
function Modes.set_major_mode(doc, mode)
    doc.properties["major_mode"] = mode
end

--- Gets the stack of Minor Modes of a Document.
--- @param doc Core.Document
--- @return Core.Mode[]
function Modes.get_minor_modes(doc)
    local modes = doc.properties["minor_modes"]
    if not modes or type(modes) ~= "table" then
        return {}
    end

    local resolved = {}
    for _, item in ipairs(modes) do
        local mode = Modes.get_mode(item)
        if mode then
            table.insert(resolved, mode)
        end
    end

    return resolved
end

--- Adds a Minor Mode to a Document.
--- @param doc Core.Document
--- @param mode string
function Modes.add_minor_mode(doc, mode)
    if Modes.has_minor_mode(doc, mode) then
        return
    end

    local modes = doc.properties["minor_modes"]
    if not modes or type(modes) ~= "table" then
        modes = {}
    end
    table.insert(modes, mode)

    doc.properties["minor_modes"] = modes
end

--- Removes a Minor Mode from a Document.
--- @param doc Core.Document
--- @param mode string
function Modes.remove_minor_mode(doc, mode)
    local modes = doc.properties["minor_modes"]
    if not modes or type(modes) ~= "table" then
        return
    end

    for idx = #modes, 1, -1 do
        local item = modes[idx]

        if mode == item then
            table.remove(modes, idx)
            break
        end
    end

    doc.properties["minor_modes"] = modes
end

--- Checks if a Document has a specific Minor Mode.
--- @param doc Core.Document
--- @param mode string
--- @return boolean
function Modes.has_minor_mode(doc, mode)
    local modes = doc.properties["minor_modes"]
    if not modes or type(modes) ~= "table" then
        return false
    end

    for _, item in ipairs(modes) do
        if item == mode then
            return true
        end
    end

    return false
end

--- Gets the Minor Mode Override for a Document.
--- @param doc Core.Document
--- @return Core.Mode?
function Modes.get_minor_mode_override(doc)
    return Modes.get_mode(doc.properties["minor_mode_override"])
end

--- Sets the Minor Mode Override for a Document.
--- @param doc Core.Document
--- @param mode string
function Modes.set_minor_mode_override(doc, mode)
    doc.properties["minor_mode_override"] = mode
end

--- Removes the Minor Mode Override from a Document.
--- @param doc Core.Document
function Modes.remove_minor_mode_override(doc)
    doc.properties["minor_mode_override"] = nil
end

return Modes
