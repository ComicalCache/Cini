--- @meta

--- @class Version
--- @field name string The application name.
--- @field version string The application version.
--- @field build_date string The build timestamp.
--- @field build_type string The build configuration.
local VersionClass = {}

--- The global version information singleton.
--- @type Version
--- @diagnostic disable-next-line: missing-fields
Version = {}
