--- @meta

--- Viewports abstract Display regions occupied by a Window. They draw a Document and optional gutter and mode line.
--- @class Core.Viewport
--- @field view Core.DocumentView View of the to displayable Document
Core.Viewport = {}

--- Changes the displayed DocumentView.
--- @param view Core.DocumentView
function Core.Viewport:change_document_view(view) end

--- Adjusts the Viewport to correctly display the cursor.
function Core.Viewport:adjust() end

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
