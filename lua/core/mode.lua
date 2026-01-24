--- @class Core.Mode
--- @field name string The name of the mode.
--- @field keymap? table<string, function|table> Keybindings for this mode.
--- @field faces? table<string, Core.Face> Face definitions for this mode.
local Mode = {}

--- @class Core.ModeOptions
--- @field name string
--- @field keymap? table<string, function|table>
--- @field faces? table<string, Core.Face>
local CoreOptions = {}

function Mode.init()
    Core.Mode = Mode
end

--- @param options? Core.ModeOptions
--- @return Core.Mode
function Mode.new(options)
    local self = setmetatable({}, Mode)

    if options then
        self.name = options.name
        self.keymap = options.keymap or {}
        self.faces = options.faces or {}
    end

    return self
end

return Mode
