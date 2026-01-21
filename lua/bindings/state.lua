--- @meta

--- @class State
--- @field editor Core.Editor The active editor instance.
--- @field name string The application name.
--- @field version string The application version.
--- @field build_date string The build timestamp.
--- @field build_type string The build configuration.
local StateClass = {}

--- The global application state singleton.
--- @type State
--- @diagnostic disable-next-line: missing-fields
State = {}
