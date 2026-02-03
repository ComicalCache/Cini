--- @meta

--- @class Cini
--- @field documents Core.Document[] The opened Documents.
--- @field workspace Core.Workspace The workspace of the Editor.
local CiniClass = {}

--- Creates a new Document.
--- @param path string? File path of the backing file.
--- @return Core.Document
function CiniClass:create_document(path) end

--- Destroys an existing Document, replacing all used instances with a scratchpad Document.
--- @param doc Core.Document
function CiniClass:destroy_document(doc) end

--- Sets a status message.
--- @param message string
--- @param mode string The mode of the status message.
--- @param ms integer The amount of milliseconds to display the status message.
--- @param force_viewport boolean Force the message (even if short) to be displayed in its own Viewport.
function CiniClass:set_status_message(message, mode, ms, force_viewport) end

--- Clears the status message in the Mini Buffer.
function CiniClass:clear_status_message() end

--- The global Cini singleton.
--- @type Cini
--- @diagnostic disable-next-line: missing-fields
Cini = {}
