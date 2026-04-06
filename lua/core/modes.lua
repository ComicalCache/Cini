--- @class Core.Modes
local Modes = {}

--- @class Core.Mode
--- @field name string The name of the mode.
--- @field keymap? table<string, string|table> Keybindings for this mode.
--- @field faces? table<string, Core.Face|string> Face definitions for this mode.
--- @field cursor_style? Core.CursorStyle Cursor style for this mode.
--- @field mode_line? fun(viewport: Core.Viewport): table Mode line for this mode.
local Mode = {}

--- Global mode registry.
--- @type table<string, Core.Mode>
Modes.modes = {}

function Modes.init()
    Core.Modes = Modes
end

--- Registers a named mode.
--- @param mode Core.Mode
function Modes.register_mode(mode)
    Modes.modes[mode.name] = mode
end

--- Retrieves a mode by name.
--- @param mode string
--- @return Core.Mode?
function Modes.get_mode(mode)
    return Modes.modes[mode]
end

--- Gets the Major Mode of a Document or DocumentView.
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

--- Gets the stack of Minor Modes of a Document or DocumentView.
--- @param doc Core.Document|Core.DocumentView
--- @return Core.Mode[]
function Modes.get_minor_modes(doc)
    local modes = doc.properties["minor_modes"]
    if not modes or type(modes) ~= "table" then return {} end

    local resolved = {}
    for _, item in ipairs(modes) do
        local mode = Modes.get_mode(item)
        if mode then
            table.insert(resolved, mode)
        end
    end

    return resolved
end

--- Adds a Minor Mode to a Document or DocumentView.
--- @param doc Core.Document|Core.DocumentView
--- @param mode string
function Modes.add_minor_mode(doc, mode)
    if Modes.has_minor_mode(doc, mode) then return end

    local modes = doc.properties["minor_modes"]
    if not modes or type(modes) ~= "table" then modes = {} end
    table.insert(modes, mode)

    doc.properties["minor_modes"] = modes
end

--- Removes a Minor Mode from a Document or DocumentView.
--- @param doc Core.Document|Core.DocumentView
--- @param mode string
function Modes.remove_minor_mode(doc, mode)
    local modes = doc.properties["minor_modes"]
    if not modes or type(modes) ~= "table" then return end

    for idx = #modes, 1, -1 do
        local item = modes[idx]

        if mode == item then
            table.remove(modes, idx)
            break
        end
    end

    doc.properties["minor_modes"] = modes
end

--- Checks if a Document or DocumentView has a specific Minor Mode.
--- @param doc Core.Document|Core.DocumentView
--- @param mode string
--- @return boolean
function Modes.has_minor_mode(doc, mode)
    local modes = doc.properties["minor_modes"]
    if not modes or type(modes) ~= "table" then return false end

    for _, item in ipairs(modes) do
        if item == mode then return true end
    end

    return false
end

--- Gets the Minor Mode Override for a DocumentView.
--- @param view Core.DocumentView
--- @return Core.Mode?
function Modes.get_minor_mode_override(view)
    return Modes.get_mode(view.properties["minor_mode_override"])
end

--- Sets the Minor Mode Override for a DocumentView.
--- @param view Core.DocumentView
--- @param mode string
function Modes.set_minor_mode_override(view, mode)
    view.properties["minor_mode_override"] = mode
end

--- Removes the Minor Mode Override from a DocumentView.
--- @param view Core.DocumentView
function Modes.remove_minor_mode_override(view)
    view.properties["minor_mode_override"] = nil
end

--- Resolves a cursor style for a specific DocumentView. This function must never modify text properties. Failure to do
--- so can result in UB and crashes.
---
--- Cursor styles are searched in a hierarchy:
--- 1. Document Minor Mode Override
--- 2. Document Minor Modes
--- 3. Document Major Mode
--- 4. Core.CursorStyle.SteadyBlock
--- @param view Core.DocumentView
--- @return integer
function Modes.resolve_cursor_style(view)
    -- 1. Minor Mode Override.
    local override = Modes.get_minor_mode_override(view)
    if override and override.cursor_style then return override.cursor_style end

    -- 2. Minor Modes.
    local minor_modes = Modes.get_minor_modes(view)
    for idx = #minor_modes, 1, -1 do
        local mode = minor_modes[idx]
        if mode.cursor_style then return mode.cursor_style end
    end

    -- 3. Major Mode.
    local major_mode = Modes.get_major_mode(view.doc)
    if major_mode and major_mode.cursor_style then return major_mode.cursor_style end

    -- Default.
    return Core.CursorStyle.SteadyBlock
end

return Modes
