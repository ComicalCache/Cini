--- @meta

--- A DocumentView acts as a middle-man between Viewport and Document.
--- @class Core.DocumentView
--- @field doc Core.Document Displayed Document. This never changes.
--- @field cur Core.Cursor
--- @field properties table<string, any> View attached properties.
--- The following properties serve a specific function:
---     - C++-Core:
---         - "ws": whitespace replacement character
---         - "nl": newline replacement character
---         - "tab": tab replacement character
---         - "tab_width": width of the tab character
---         - "minor_modes": stack of Minor Modes of the DocumentView
---         - "minor_mode_override": Mode to (temporary) override all Minor Modes
---         - "loaded": the DocumentView is currently displayed
---     - Lua-Core:
---         - "name": displayable name
Core.DocumentView = {}

--- Moves the Cursor using a Cursor move function.
--- @param move_func fun(cursor: Core.Cursor, view: Core.DocumentView, n: integer)
--- @param n integer
--- @return boolean If the cursor moved.
function Core.DocumentView:move_cursor(move_func, n) end

--- Add or update view properties on a text range.
--- The following properties serve a specific purpose:
---     - "replacement": replacement string that is displayed instead. Replacements must *never* be zero-width! Failure
---         to do so will cause in an infinite loop.
---     - "hover_action": a function called when the cursor is on the text property.
---     - "selection": a text selection
---
--- The evaluation order of faces is defined in Cini.face_layers
--- @param start integer
--- @param stop integer
--- @param key string
--- @param value any
function Core.DocumentView:add_view_property(start, stop, key, value) end

--- Removes view properties from a text range.
--- @param start integer
--- @param stop integer
--- @param key string
function Core.DocumentView:remove_view_property(start, stop, key) end

--- Removes all (matching) view properties from the Document.
--- @param key? string
function Core.DocumentView:clear_view_properties(key) end

--- Merges matching overlapping view properties.
--- @param key string
function Core.DocumentView:optimize_view_properties(key) end

--- Returns the matching view property at a point.
--- @param pos integer
--- @param key string
--- @return any
function Core.DocumentView:get_view_property(pos, key) end

--- Returns a table of all view properties at a point.
--- @param pos integer
--- @return table<string, any>
function Core.DocumentView:get_view_properties(pos) end

--- Returns a list of all view properties with a specific key.
--- @param key string
--- @return table<integer, any>
function Core.DocumentView:get_all_view_properties(key) end
