local M = {}

-- State for multi-key sequences.
M.pending_map = nil
M.pending_keys = {}

function M.init()
    Core.on_input = M.on_input
end

function M.on_input(editor, key)
    local key_str = key:to_string()

    local maps = {}
    if M.pending_map then
        maps = { M.pending_map }
    else
        maps = M.fetch_keymaps(editor)
    end

    local match = nil
    for _, map in ipairs(maps) do
        if map[key_str] then
            match = map[key_str]
            break
        end
    end

    if not match then
        if M.pending_map then
            local sequence = table.concat(M.pending_keys, " ") .. " " .. key_str
            -- TODO: show info.
            M.pending_map = nil
            M.pending_keys = {}
        else
            -- TODO: show info.
        end

        return
    end

    if type(match) == "function" then
        match(editor)
        M.pending_map = nil
        M.pending_keys = {}
    elseif type(match) == "table" then
        table.insert(M.pending_keys, key_str)
        M.pending_map = match
        -- TODO: show current sequence in mode line?
    end
end

function M.fetch_keymaps(editor)
    local Mode = require("core.internals.mode")

    local doc = editor.viewport.doc
    local maps = {}

    -- 1. Text properties.
    local prop_keymap = doc:get_text_property(editor.viewport.cursor:point(doc), "keymap")
    if prop_keymap then
        table.insert(maps, prop_keymap)
    end

    -- 2. Document Minor Mode Override.
    local override = Mode.get_minor_mode_override(doc)
    if override and override.keymap then
        table.insert(maps, override.keymap)
    end

    -- 3. Document Minor Modes.
    local minor_modes = Mode.get_minor_modes(doc)
    for idx = #minor_modes, 1, -1 do
        local mode = minor_modes[idx]
        if mode.keymap then
            table.insert(maps, mode.keymap)
        end
    end

    -- 4. Document Major Mode.
    local major_mode = Mode.get_major_mode(doc)
    if major_mode and major_mode.keymap then
        table.insert(maps, major_mode.keymap)
    end

    -- 5. Global Mode.
    local global_mode = Mode.get_mode("global")
    if global_mode and global_mode.keymap then
        table.insert(maps, global_mode.keymap)
    end

    return maps
end

function M.bind(mode_name, sequence, action)
    local Mode = require("core.internals.mode")

    local keys = {}
    for key in sequence:gmatch("%S+") do
        table.insert(keys, Core.Key.normalize(key))
    end

    if #keys == 0 then return end

    -- Get or create the mode.
    local mode = Mode.get_mode(mode_name)
    if not mode then
        mode = { name = mode_name, keymap = {} }
        Mode.register_mode(mode_name, mode)
    end

    -- Ensure keymap exists
    if not mode.keymap then
        mode.keymap = {}
    end

    -- Build the nested keymap tree.
    local current_map = mode.keymap
    for idx = 1, #keys - 1 do
        local key = keys[idx]
        if not current_map[key] then
            current_map[key] = {}
        elseif type(current_map[key]) ~= "table" then
            -- Overwriting a previous single-key binding with a prefix.
            current_map[key] = {}
        end
        current_map = current_map[key]
    end

    current_map[keys[#keys]] = action
end

return M
