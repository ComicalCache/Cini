local home = "$HOME_DIR" -- Replace this with your absolute home directory path.

local modules = {
    dofile(home .. "/.config/cini/config/insert.lua"),
    dofile(home .. "/.config/cini/config/mini_buffer.lua"),
}

local UserConfig = {}

function UserConfig.setup()
    for _, m in ipairs(modules) do if m.setup then m.setup() end end
end

function UserConfig.init()
    for _, m in ipairs(modules) do if m.init then m.init() end end
end

return UserConfig
