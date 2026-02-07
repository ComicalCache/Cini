--- @class Core.Faces
local Faces = {}

-- Global face registry.
--- @type table<string, Core.Face>
Faces.faces = {}

function Faces.init()
    Core.Faces = Faces
end

--- Registers a named face.
--- @param name string
--- @param face Core.Face
function Faces.register_face(name, face)
    Faces.faces[name] = face
end

--- Retrieves a face by name.
--- @param name string
--- @return Core.Face?
function Faces.get_face(name)
    return Faces.faces[name]
end

--- Resolves a face for a specific Document. Faces are searched in a hierarchy:
--- 1. Document Minor Mode Override
--- 2. Document Minor Modes
--- 3. Document Major Mode
--- 4. Global Registry
--- @param doc Core.Document
--- @param name string
--- @return Core.Face?
function Faces.resolve_face(doc, name)
    -- 1. Document Minor Mode Override.
    local override = Core.Modes.get_minor_mode_override(doc)
    if override and override.faces and override.faces[name] then
        return override.faces[name]
    end

    -- 2. Document Minor Modes.
    local minor_modes = Core.Modes.get_minor_modes(doc)
    for idx = #minor_modes, 1, -1 do
        local mode = minor_modes[idx]
        if mode.faces and mode.faces[name] then
            return mode.faces[name]
        end
    end

    -- 3. Document Major Mode.
    local major_mode = Core.Modes.get_major_mode(doc)
    if major_mode and major_mode.faces and major_mode.faces[name] then
        return major_mode.faces[name]
    end

    -- 4. Global Registry.
    return Faces.get_face(name)
end

return Faces
