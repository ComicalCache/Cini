local M = {}

-- Global face registry.
---@type table<string, Core.Face>
M.faces = {}

function M.post_init()
    State.editor.viewport:set_get_face(function(doc, name)
        return M.resolve_face(doc, name)
    end)

    State.editor.mini_buffer:set_get_face(function(doc, name)
        return M.resolve_face(doc, name)
    end)
end

--- @param name string
--- @param face Core.Face
function M.register_face(name, face)
    M.faces[name] = face
end

--- @param name string
--- @return Core.Face?
function M.get_face(name)
    return M.faces[name]
end

--- @param doc Core.Document
--- @param name string
--- @return Core.Face?
function M.resolve_face(doc, name)
    local Mode = require("core.mode")

    -- 1. Document Minor Mode Override.
    local override = Mode.get_minor_mode_override(doc)
    if override and override.faces and override.faces[name] then
        return override.faces[name]
    end

    -- 2. Document Minor Modes.
    local minor_modes = Mode.get_minor_modes(doc)
    for idx = #minor_modes, 1, -1 do
        local mode = minor_modes[idx]
        if mode.faces and mode.faces[name] then
            return mode.faces[name]
        end
    end

    -- 3. Document Major Mode.
    local major_mode = Mode.get_major_mode(doc)
    if major_mode and major_mode.faces and major_mode.faces[name] then
        return major_mode.faces[name]
    end

    -- 4. Global Registry.
    return M.get_face(name)
end

return M
