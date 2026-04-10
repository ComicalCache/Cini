--- @class Core.Clone
local Clone = {}

function Clone.init()
    Core.Clone = Clone
end

--- Deepclones a lua object.
--- @param obj any
--- @return any
function Clone.deepclone(obj)
    local copy

    if type(obj) == 'table' then
        copy = {}

        for obj_key, obj_value in next, obj, nil do
            copy[Clone.deepclone(obj_key)] = Clone.deepclone(obj_value)
        end

        setmetatable(copy, Clone.deepclone(getmetatable(obj)))
    else
        copy = obj
    end

    return copy
end

return Clone
