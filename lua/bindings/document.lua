--- @meta

--- Documents serve as the central abstraction of data.
--- @class Core.Document
--- @field point integer The current point in the Document.
--- @field properties table Document attached properties (ws, nl, tab, tab_width, modes).
--- The following properties serve a specific function:
---     - "ws": whitespace replacement character
---     - "nl": newline replacement character
---     - "tab": tab replacement character
---     - "tab_width": width of the tab character
---     - "major_mode": Major Mode of the Document
---     - "minor_modes": stack of Minor Modes of the Document
---     - "minor_mode_override": Mode to (temporary) override all Minor Modes
--- @field path string? The backing file of the Document.
--- @field size integer The size in bytes of the data in the Document.
Core.Document = {}

--- Sets the point of the Document.
--- @param point integer
function Core.Document:set_point(point) end

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

--- Returns matches for a regex pattern over the entire Document.
--- @param pattern string
--- @return Core.RegexMatch[]
function Core.Document:search(pattern) end

--- Returns matches for a regex pattern starting at the current point.
--- @param pattern string
--- @return Core.RegexMatch[]
function Core.Document:search_forward(pattern) end

--- Returns matches for a regex pattern up to the current point.
--- @param pattern string
--- @return Core.RegexMatch[]
function Core.Document:search_backward(pattern) end

--- Add or update properties on a text range.
--- The following properties serve a specific function:
---     - "face": color face of the text range
---     - "replacement": replacement string that is displayed instead
---     - "keymap": keybinds that are set for the text range
---     - "hover_action": a function called when the cursor is on the text property
--- @param start integer
--- @param stop integer
--- @param key string
--- @param value any
function Core.Document:add_text_property(start, stop, key, value) end

--- Removes properties from a text range.
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

--- Returns the matching property at a point.
--- @param point integer
--- @param key string
--- @return any
function Core.Document:get_text_property(point, key) end

--- Returns a table of all properties at a point.
--- @param point integer
--- @return table
function Core.Document:get_text_properties(point) end
