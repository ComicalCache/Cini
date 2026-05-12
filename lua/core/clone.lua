--- @class Core.Clone
local Clone = {}

function Clone.init()
    Core.Clone = Clone
end

--- Deepclones a lua object if possible.
--- @param obj any
--- @return any
function Clone.deepclone(obj)
    if obj == nil then return nil end

    local copy

    if type(obj) == 'table' then
        copy = {}

        for obj_key, obj_value in next, obj, nil do
            local cloned_key = Clone.deepclone(obj_key)
            if cloned_key ~= nil then
                copy[cloned_key] = Clone.deepclone(obj_value)
            end
        end

        local mt = getmetatable(obj)
        if mt then setmetatable(copy, Clone.deepclone(mt)) end
    elseif type(obj) == 'userdata' then
        local mt = getmetatable(obj)

        local success, clone_func = pcall(function()
            if mt and type(mt.__index) == 'table' and type(mt.__index.clone) == 'function' then
                return mt.__index.clone
            end

            return obj.clone
        end)

        if success and type(clone_func) == 'function' then
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
