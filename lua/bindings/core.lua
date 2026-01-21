--- @meta

--- The global namespace for the editor Core API.
--- All C++ bindings are attached to this table.
--- @class Core
Core = {}

-- Explicitly tell the LSP that 'Core' is a global variable.
_G.Core = Core
