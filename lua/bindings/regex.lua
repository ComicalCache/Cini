--- @meta

--- @class Core.Regex
Core.Regex = {}

--- @param pattern string
--- @return Core.Regex
function Core.Regex.new(pattern) end

--- Searches a text and returns all matches.
--- @param text string
--- @return Core.RegexMatch[]
function Core.Regex:search(text) end
