--- @meta

--- The global namespace for the editor Core API.
--- All C++ bindings are attached to this table.
--- @class Core
--- @field AsyncProcess Core.AsyncProcess
--- @field Commands Core.Commands
--- @field Faces Core.Faces
--- @field Hooks Core.Hooks
--- @field HoverActions Core.HoverActions
--- @field Keybinds Core.Keybinds
--- @field Modes Core.Modes
--- @field Motions Core.Motions
--- @field Prompt Core.Prompt
--- @field Clipboard Core.Clipboard
Core = {}

-- Explicitly tell the LSP that 'Core' is a global variable.
_G.Core = Core
