--- @class Core.Faces
local Faces = {}

--- Global face registry.
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

--- Resolves a face for a specific Document. This function must never modify text properties. Failure to do so can
--- result in UB and crashes.
---
--- Faces are searched in a hierarchy:
--- 1. DocumentView Minor Mode Override
--- 2. DocumentView Minor Modes
--- 3. Document Minor Modes
--- 4. Document Major Mode
--- 5. Global Registry
--- @param view Core.DocumentView
--- @param name string
--- @return Core.Face?
function Faces.resolve_face(view, name)
    local function resolve(mode)
        if mode and mode.faces and mode.faces[name] then
            local res = mode.faces[name]
            return type(res) == "string" and Faces.get_face(res) or res
        end

        return nil
    end

    -- 1. DocumentView Minor Mode Override.
    local override = resolve(Core.Modes.get_minor_mode_override(view))
    if override then return override end

    -- 2. DocumentView Minor Modes.
    local view_minor_modes = Core.Modes.get_minor_modes(view)
    for idx = #view_minor_modes, 1, -1 do
        local res = resolve(view_minor_modes[idx])
        if res then return res end
    end

    -- 3. Document Minor Modes.
    local doc_minor_modes = Core.Modes.get_minor_modes(view.doc)
    for idx = #doc_minor_modes, 1, -1 do
        local res = resolve(doc_minor_modes[idx])
        if res then return res end
    end

    -- 4. Document Major Mode.
    local major_mode = resolve(Core.Modes.get_major_mode(view.doc))
    if major_mode then return major_mode end

    -- 5. Global Registry.
    return Faces.get_face(name)
end

return Faces
