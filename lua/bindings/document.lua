--- @meta

--- Documents serve as the central abstraction of data.
--- @class Core.Document
--- @field properties table Document attached properties.
--- The following properties serve a specific function:
---     - C++-Core:
---         - "major_mode": Major Mode of the Document
---         - "minor_modes": Stack of Minor Modes of the Document
---         - "loaded": The Document is currently being displayed by one or more DocumentViews
---         - "process_attached": The Document has a process attached to it that outputs its content into it.
---     - Lua-Core:
---         - "name": displayable name
--- @field path string? The backing file of the Document.
--- @field size integer The size in bytes of the data in the Document.
--- @field lines integer The count of lines in the Document.
--- @field modified boolean If the Document contains unsaved changes.
Core.Document = {}

--- Returns all DocumentViews holding this Document.
--- @return table<integer, Core.DocumentView>
function Core.Document:views() end

--- Writes the contents to the underlying or new path.
--- @param path string? File path to write to.
function Core.Document:save(path) end

--- Inserts data at a point into the Document.
--- @param point integer
--- @param text string
function Core.Document:insert(point, text) end

--- Removes a range of data from the Document.
--- @param start integer
--- @param stop integer
function Core.Document:remove(start, stop) end

--- Clears the entire Document.
function Core.Document:clear() end

--- Replaces a range from start to end with data.
--- @param start integer
--- @param stop integer
--- @param data string
function Core.Document:replace(start, stop, data) end

--- Returns the nth line of the Document.
--- @param n integer
--- @return string
function Core.Document:line(n) end

--- Returns a range of data from start to end form.
--- @param start integer
--- @param stop integer
--- @return string
function Core.Document:slice(start, stop) end

--- Returns the byte beginning the nth line of the document.
--- @param nth integer
--- @return integer
function Core.Document:line_begin_byte(nth) end

--- Returns the byte one after the end of the nth line of the document. This includes the newline character.
--- @param nth integer
--- @return integer
function Core.Document:line_end_byte(nth) end

--- Converts a byte offset into a position.
--- @param byte integer
--- @return Core.Position
function Core.Document:position_from_byte(byte) end

--- Returns matches for a regex pattern over a Document range. Only supply the Regex if you want to use the default
--- arguments.
--- @param regex Core.Regex
--- @param start integer (defaults to 0)
--- @param stop integer (defaults to the length of the Document)
--- @return Core.RegexMatch[]
function Core.Document:search(regex, start, stop) end

--- Begins a transaction to undo/redo.
--- @param point integer The current cursor point.
function Core.Document:begin_transaction(point) end

--- Ends a transaction to undo/redo.
--- @param point integer The current cursor point.
function Core.Document:end_transaction(point) end

--- Undos the last transaction.
--- @return nil|integer The cursor position or nil if nothing to undo.
function Core.Document:undo() end

--- Redos the last transaction.
--- @return nil|integer The cursor position or nil if nothing to redo.
function Core.Document:redo() end

--- Add or update text properties on a text range.
--- The following properties serve a specific function:
---     - "keymap": keybinds that are set for the text range.
---
--- The evaluation order of faces is defined in Cini.face_layers
--- @param start integer
--- @param stop integer
--- @param key string
--- @param value any
function Core.Document:add_text_property(start, stop, key, value) end

--- Removes text properties from a text range.
--- @param start integer
--- @param stop integer
--- @param key string
function Core.Document:remove_text_property(start, stop, key) end

--- Removes all (matching) text properties from the Document.
--- @param key? string
function Core.Document:clear_text_properties(key) end

--- Merges matching overlapping text properties.
--- @param key string
function Core.Document:optimize_text_properties(key) end

--- Returns the matching text property at a point.
--- @param point integer
--- @param key string
--- @return any
function Core.Document:get_text_property(point, key) end

--- Returns a table of all text properties at a point.
--- @param point integer
--- @return table<string, any>
function Core.Document:get_text_properties(point) end

--- Returns a list of all text properties with a specific key.
--- @param key string
--- @return table[]
function Core.Document:get_all_text_properties(key) end
