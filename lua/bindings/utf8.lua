--- @meta

--- @class Core.Utf8
Core.Utf8 = {}

--- Returns the count of unicode chars in a UTF-8 string.
--- @param str string
--- @return integer
function Core.Utf8.count(str) end

--- Returns the lenght (byte count) of a unicode character.
--- @param ch string the char (the first byte is used for calculation).
--- @return integer
function Core.Utf8.len(ch) end
