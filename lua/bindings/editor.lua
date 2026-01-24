--- @meta

--- @class Core.Editor
--- @field viewport Core.Viewport The currently active viewport.
--- @field mini_buffer Core.Viewport The Mini Buffer viewport.
Core.Editor = {}

--- Closes the current viewport.
function Core.Editor:quit() end

--- Sets a status message.
--- @param message string
--- @param force_viewport boolean Force the message (even if short) to be displayed in its own Viewport.
function Core.Editor:set_status_message(message, force_viewport) end

--- Enters the Mini Buffer.
function Core.Editor:enter_mini_buffer() end

--- Exits the Mini Buffer.
function Core.Editor:exit_mini_buffer() end

--- Splits the current Viewport vertically.
--- @param ratio number
function Core.Editor:split_vertical(ratio) end

--- Splits the current Viewport horizontally.
--- @param ratio number
function Core.Editor:split_horizontal(ratio) end

--- Resizes the current Viewport split.
--- @param delta number
function Core.Editor:resize_split(delta) end

--- Changes the current Viewport.
--- @param direction Core.Direction
function Core.Editor:navigate_splits(direction) end
