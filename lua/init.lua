-- Startup is split into two parts:
-- 1. Setup, sets up static data in the following order:
--      1. Faces
--      2. Modes
--      3. Mode Line.
--      4. Hooks
--      5. Commands
--      6. Keybinds
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
require("core.mode_line").init()
require("core.modes").init()
require("core.motions").init()
require("core.prompt").init()
require("core.quit").init()
require("core.util").init()

local modules = {
    require("default.dired"),
    require("default.document_viewer"),
    require("default.global"),
    require("default.insert"),
    require("default.mini_buffer"),
    require("default.mode_line"),
    require("default.pager"),
    require("default.replace"),
    require("default.search"),
    require("default.selection"),
}

for _, m in ipairs(modules) do if m.setup then m.setup() end end
if User and User.Config and User.Config.setup then User.Config.setup() end
for _, m in ipairs(modules) do if m.init then m.init() end end
if User and User.Config and User.Config.init then User.Config.init() end

Cini.face_layers = { "face", "search", "selection" }
