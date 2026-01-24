--- @meta

--- The global namespace for the editor Core API.
--- All C++ bindings are attached to this table.
--- @class Core
--- @field Faces Core.Faces
--- @field Hooks Core.Hooks
--- @field Keybinds Core.Key
--- @field Mode Core.Mode
--- @field Modes Core.Modes
Core = {}

-- Explicitly tell the LSP that 'Core' is a global variable.
_G.Core = Core
