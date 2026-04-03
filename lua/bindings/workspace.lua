--- @meta

--- A Workspace manages a tiling tree *including* a MiniBuffer.
--- @class Core.Workspace
--- @field is_mini_buffer boolean If current the Mini Buffer is active.
--- @field viewport Core.Viewport The currently active viewport.
--- @field mini_buffer Core.Viewport The Mini Buffer viewport.
Core.Workspace = {}

--- Focuses a specific Viewport.
--- @param viewport Core.Viewport
function Core.Workspace:focus_viewport(viewport) end

--- Closes a specific Viewport.
--- @param viewport Core.Viewport
--- @return boolean true if the last viewport was closed
function Core.Workspace:close_viewport(viewport) end

--- Searches the Window tree for the first Viewport matching a predicate.
--- @param predicate fun(viewport: Core.Viewport): boolean
--- @return Core.Viewport?
function Core.Workspace:find_viewport(predicate) end

--- Enters the Mini Buffer.
function Core.Workspace:enter_mini_buffer() end

--- Exits the Mini Buffer.
function Core.Workspace:exit_mini_buffer() end

--- Splits the current Viewport vertically.
--- @param ratio number
function Core.Workspace:split_vertical(ratio) end

--- Splits the current Viewport horizontally.
--- @param ratio number
function Core.Workspace:split_horizontal(ratio) end

--- Resizes the current Viewport split.
--- @param delta number
function Core.Workspace:resize_split(delta) end

--- Changes the current Viewport.
--- @param direction Core.Direction
function Core.Workspace:navigate_split(direction) end

--- Closes the current viewport.
--- @return boolean true if the last viewport was closed
function Core.Workspace:close_split() end
