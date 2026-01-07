local open_document = require("core.callbacks.open_document")

require("core.internals.bind").setup()

require("core.keymap.global").setup()
require("core.keymap.insert").setup()
require("core.keymap.mini_buffer").setup()

require("core.theme").setup()

require("modes.cpp").setup()
require("modes.status_message").setup()
require("modes.text").setup()
require("modes.whitespace").setup()

Core.Document.set_open_callback(open_document.callback)
