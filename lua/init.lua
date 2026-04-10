-- Startup is split into two parts:
-- 1. Setup, sets up static data in the following order:
--      1. Faces
--      2. Modes
--      3. Hooks
--      4. Commands
--      5. Keybinds
-- 2. Init, sets up dynamic data (anything that requires hooks to listen):
--      1. Motions
--      2. Hover Actions

-- Initialize core library features.
-- Hooks are required by many other systems and thus need to be initialized early.
require("core.hooks").init()

require("core.clone").init()
require("core.commands").init()
require("core.faces").init()
require("core.hover_actions").init()
require("core.keybinds").init()
require("core.modes").init()
require("core.motions").init()
require("core.prompt").init()
require("core.quit").init()

local modules = {
    require("default.modes.document_viewer"),
    require("default.modes.global"),
    require("default.modes.insert"),
    require("default.modes.mini_buffer"),
    require("default.modes.pager"),
    require("default.modes.selection"),
    require("default.mode_line"),
}

for _, m in ipairs(modules) do if m.setup then m.setup() end end
for _, m in ipairs(modules) do if m.init then m.init() end end

Cini.face_layers = { "face", "selection" }
