--- @meta

--- @class Core.Viewport
--- @field doc Core.Document The Document of the Viewport.
--- @field cursor Core.Cursor The Cursor of the Viewport.
Core.Viewport = {}

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

--- Sets the get_face callback to retrieve a face by name.
--- @param callback fun(doc: Core.Document, name: string): Core.Face?
function Core.Viewport:set_get_face(callback) end

--- Sets the get_face_at callback to retrieve a face from a text property.
--- @param callback fun(doc: Core.Document, point: integer): Core.Face?
function Core.Viewport:set_get_face_at(callback) end
