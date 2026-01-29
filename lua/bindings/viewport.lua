--- @meta

--- @class Core.Viewport
--- @field doc Core.Document The Document of the Viewport.
--- @field cursor Core.Cursor The Cursor of the Viewport.
Core.Viewport = {}

--- Changes the Document displayed by the Viewport.
--- @param doc Core.Document
function Core.Viewport:change_document(doc) end

--- Moves the Cursor using a Cursor move function.
--- @param move_func fun(cursor: Core.Cursor, doc: Core.Document, n: integer)
--- @param n integer
function Core.Viewport:move_cursor(move_func, n) end

--- Toggles the gutter.
function Core.Viewport:toggle_gutter() end

--- Configures the Mode Line.
--- Callback returns a list of { text="...", face="..." } or { spacer=true }.
--- @param callback fun(): table[]
function Core.Viewport:set_mode_line(callback) end

--- Toggles the Mode Line.
function Core.Viewport:toggle_mode_line() end

--- Moves the Viewport up.
--- @param n integer
function Core.Viewport:scroll_up(n) end

--- Moves the Viewport down.
--- @param n integer
function Core.Viewport:scroll_down(n) end

--- Moves the Viewport to the left.
--- @param n integer
function Core.Viewport:scroll_left(n) end

--- Moves the Viewport to the right.
--- @param n integer
function Core.Viewport:scroll_right(n) end
