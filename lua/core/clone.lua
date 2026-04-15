--- @class Core.Clone
local Clone = {}

function Clone.init()
    Core.Clone = Clone
end

--- Deepclones a lua object if possible.
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
    elseif type(obj) == 'userdata' then
        local mt = getmetatable(obj)

        -- C++ defined userdata may offer a clone function to clone C++ bound objects.
        if mt and type(mt.__index) == 'table' and type(mt.__index.clone) == 'function' then
            copy = obj:clone()
        elseif type(obj.clone) == 'function' then
            copy = obj:clone()
        else
            copy = obj
        end
    else
        copy = obj
    end

    return copy
end

return Clone
