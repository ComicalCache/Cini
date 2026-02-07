--- @meta

--- Keys abstract input keys by storing them as their Unicode codepoint and key-modifier.
--- @class Core.Key
Core.Key = {}

--- Converts a Key into its string representation.
--- @return string
function Core.Key:to_string() end

--- Normalizes a Key string.
--- @param str string
--- @return string
function Core.Key.normalize(str) end
