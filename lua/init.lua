-- Hooks are required by many other systems and thus need to be initialized early.
require("core.hooks").init()

require("core.commands").init()
require("core.faces").init()
require("core.hover_actions").init()
require("core.keybinds").init()
require("core.modes").init()
require("core.motions").init()
require("core.prompt").init()

require("default.modes.document_viewer").init()
require("default.modes.global").init()
require("default.modes.insert").init()
require("default.modes.mini_buffer").init()

require("default.mode_line").init()

Cini.face_layers = { "face", "selection" }
