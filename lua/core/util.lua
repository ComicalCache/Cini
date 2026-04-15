--- @class Core.Util
local Util = {}

function Util.init()
    Core.Util = Util
end

--- Calculates the width (as in amount of spaces) of a tab at a specific offset with a given tab width.
--- @param tab_width number
--- @param offset number
--- @return number
function Util.tab_width(tab_width, offset)
    return tab_width - math.fmod(offset, tab_width)
end

return Util
