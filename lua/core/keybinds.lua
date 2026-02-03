--- @class Core.Keybinds
local Keybinds = {}

-- State for multi-key sequences.
-- This holds the sub-keymap we are currently traversing.
--- @type table<string, function|table>?
Keybinds.pending_map = nil

--- @type string[]
Keybinds.pending_keys = {}

function Keybinds.init()
    Core.Keybinds = Keybinds
end

--- @param key Core.Key
function Keybinds.on_input(key)
    local key_str = key:to_string()

    local maps = {}
    if Keybinds.pending_map then
        maps = { Keybinds.pending_map }
    else
        maps = Keybinds.fetch_keymaps()
    end

    local matches = {}
    local leaf_match = nil

    for _, map in ipairs(maps) do
        local val = map[key_str]

        if val then
            if type(val) == "function" then
                -- A leaf binding shadows everything.

                -- If its not a prefix to other commands, its a match.
                if #matches == 0 then
                    leaf_match = val
                    break
                end
            elseif type(val) == "table" then
                table.insert(matches, val)
            end
        elseif #matches == 0 and not leaf_match and map["<CatchAll>"] then
            -- Only check CatchAll if no match was found yet.
            if map["<CatchAll>"](key_str) then
                Keybinds.pending_map = nil
                Keybinds.pending_keys = {}
                return
            end
        end
    end

    if leaf_match then
        leaf_match()
        Keybinds.pending_map = nil
        Keybinds.pending_keys = {}
        return
    end

    if #matches > 0 then
        -- Merge all matching sub-maps into one.
        local merged = {}
        for i = #matches, 1, -1 do
            for k, v in pairs(matches[i]) do
                merged[k] = v
            end
        end

        table.insert(Keybinds.pending_keys, key_str)
        Keybinds.pending_map = merged
        -- TODO: show current sequence in mode line?
        return
    end

    -- No match found.
    if Keybinds.pending_map then
        local sequence = table.concat(Keybinds.pending_keys, " ") .. " " .. key_str
        Cini:set_status_message("Undefined sequence: " .. sequence, "info_message", 2000, false)

        Keybinds.pending_map = nil
        Keybinds.pending_keys = {}
    else
        Cini:set_status_message("Undefined key: " .. key_str, "info_message", 2000, false)
    end
end

--- @return table[]
function Keybinds.fetch_keymaps()
    local doc = Cini.workspace.is_mini_buffer and Cini.workspace.mini_buffer.doc or Cini.workspace.viewport.doc
    local cursor = Cini.workspace.is_mini_buffer and Cini.workspace.mini_buffer.cursor or Cini.workspace.viewport.cursor
    local maps = {}

    -- 1. Text properties.
    local property_keymap = doc:get_text_property(cursor:point(doc), "keymap")
    if property_keymap then
        table.insert(maps, property_keymap)
    end

    -- 2. Document Minor Mode Override.
    local override = Core.Modes.get_minor_mode_override(doc)
    if override and override.keymap then
        table.insert(maps, override.keymap)
    end

    -- 3. Document Minor Modes.
    local minor_modes = Core.Modes.get_minor_modes(doc)
    for idx = #minor_modes, 1, -1 do
        local mode = minor_modes[idx]
        if mode.keymap then
            table.insert(maps, mode.keymap)
        end
    end

    -- 4. Document Major Mode.
    local major_mode = Core.Modes.get_major_mode(doc)
    if major_mode and major_mode.keymap then
        table.insert(maps, major_mode.keymap)
    end

    -- 5. Global Mode.
    local global_mode = Core.Modes.get_mode("global")
    if global_mode and global_mode.keymap then
        table.insert(maps, global_mode.keymap)
    end

    return maps
end

---@param mode string|Core.Mode
---@param sequence string
---@param action fun()|fun(key_str: string): boolean The function to execute.
function Keybinds.bind(mode, sequence, action)
    -- Get or create the mode.
    if type(mode) == "string" then
        local fetched_mode = Core.Modes.get_mode(mode)
        if not fetched_mode then
            local new_mode = { name = fetched_mode, keymap = {} }
            Core.Modes.register_mode(mode, new_mode)

            mode = new_mode
        else
            mode = fetched_mode
        end
    end

    -- Ensure keymap exists
    if not mode.keymap then
        mode.keymap = {}
    end

    local keys = {}
    for key in sequence:gmatch("%S+") do
        table.insert(keys, key)
    end

    if #keys == 0 then return end

    -- Build the nested keymap tree.
    local current_map = mode.keymap
    -- The Linter doesn't realize current_map cannot be nil.
    --- @cast current_map -nil

    for idx = 1, #keys - 1 do
        local key = keys[idx]

        if key ~= "<CatchAll>" then
            key = Core.Key.normalize(key)
        end

        -- Overwriting a previous single-key binding with a prefix.
        if not current_map[key] or type(current_map[key]) ~= "table" then
            current_map[key] = {}
        end

        -- Since the type definition is recursive, I must manually cast it here.
        --- @cast current_map table<string, table<string, function|table>>
        current_map = current_map[key]
    end

    local key = keys[#keys]
    if key ~= "<CatchAll>" then
        key = Core.Key.normalize(key)
    end
    current_map[key] = action
end

return Keybinds
