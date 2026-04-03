--- @meta

--- The global namespace for the editor Core API.
--- All C++ bindings are attached to this table.
--- @class Core
--- @field Commands Core.Commands
--- @field Faces Core.Faces
--- @field Hooks Core.Hooks
--- @field HoverActions Core.HoverActions
--- @field Keybinds Core.Key
--- @field Modes Core.Modes
--- @field Motions Core.Motions
--- @field Prompt Core.Prompt
Core = {}

-- Explicitly tell the LSP that 'Core' is a global variable.
_G.Core = Core
