local M = {}

function M.setup()
    -- Keybind map.
    Keybind.prefixes = {}

    function Keybind.__dispatch(editor, key_map)
    editor:next_key(function(editor, key)
        local key_str = key:to_string()
        local next_action = key_map[key_str]

        if type(next_action) == "function" then
            -- Execute the defined function.
            next_action(editor)
        elseif type(next_action) == "table" then
            -- Go down the keybind tree.
            Keybind.__dispatch(editor, next_action)
        else
            -- TODO: print no keybind found.
        end
    end)
    end

    -- Create a keybind for a mode with an action.
    function Keybind.bind(mode, sequence_str, action)
    local keys = {}
    for key in sequence_str:gmatch("%S+") do table.insert(keys, key) end

    if #keys == 0 then return end

    -- Single key keybind.
    if #keys == 1 then
        Keybind.__bind(mode, keys[1], action)
        return
    end

    -- Get the keybind tree the mode or an empty map.
    Keybind.prefixes[mode] = Keybind.prefixes[mode] or {}
    local root_key = keys[1]
    local current_map = Keybind.prefixes[mode][root_key]

    -- Create a new bind if it not exists yet.
    if not current_map then
        current_map = {}
        Keybind.prefixes[mode][root_key] = current_map
        Keybind.__bind(mode, root_key, function(editor)
            Keybind.__dispatch(editor, current_map)
        end)
    end

    -- Build they keybind tree.
    for idx = 2, #keys - 1 do
        local key = keys[idx]
        current_map[key] = current_map[key] or {}
        current_map = current_map[key]
    end

    current_map[keys[#keys]] = action
    end
end

return M
