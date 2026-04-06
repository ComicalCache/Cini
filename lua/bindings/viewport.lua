--- @meta

--- Viewports abstract Display regions occupied by a Window. They draw a Document and optional gutter and mode line.
--- @class Core.Viewport
--- @field view Core.DocumentView View of the to displayable Document
Core.Viewport = {}

--- Changes the displayed DocumentView.
--- @param view Core.DocumentView
function Core.Viewport:change_document_view(view) end

--- Toggles the gutter.
function Core.Viewport:toggle_gutter() end

--- Configures the Mode Line.
--- Callback returns a list of { text="...", face="..." } or { spacer=true }.
--- @param callback fun(viewport: Core.Viewport): table[]
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
