--- @meta

--- Keys abstract input keys by storing them as their Unicode codepoint and key-modifier.
--- @class Core.KeyEvent
Core.KeyEvent = {}

--- Converts a Key into its string representation.
--- @return string
function Core.KeyEvent:to_string() end

--- Normalizes a Key string.
--- @param str string
--- @return string
function Core.KeyEvent.normalize(str) end
