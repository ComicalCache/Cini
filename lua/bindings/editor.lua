--- @meta

--- @class Core.Editor
--- @field documents Core.Document[] The opened Documents.
--- @field workspace Core.Workspace The workspace of the Editor.
Core.Editor = {}

--- Creates a new Document.
--- @param path string? File path of the backing file.
--- @return Core.Document
function Core.Editor:create_document(path) end

--- Destroys an existing Document, replacing all used instances with a scratchpad Document.
--- @param doc Core.Document
function Core.Editor:destroy_document(doc) end

--- Sets a status message.
--- @param message string
--- @param force_viewport boolean Force the message (even if short) to be displayed in its own Viewport.
function Core.Editor:set_status_message(message, force_viewport) end
