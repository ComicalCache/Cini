--- @meta

--- Namespace of the user config.
--- @class User
--- @field Config any Expected to have `setup()` and `init()`.
User = {}

-- Explicitly tell the LSP that 'User' is a global variable.
_G.User = User
