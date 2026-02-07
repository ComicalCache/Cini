--- @class Core.Hooks
local Hooks = {}

--- @type table<string, function[]>
Hooks.registry = {}

function Hooks.init()
    Core.Hooks = Hooks
end

--- Registers a callback for a specific hook.
--- The following events can be hooked:
---     - "cini::startup" | "cini::shutdown": fun()
---         after Cini was setup and before Cini is shutdown
---
---     - "cursor::before-move": fun(Core.Document, target) -> boolean
---         before the Core.Cursor is moved via Core.Viewport:move_cursor. Returning false will *prevent* the move
---             operation.
---     - "cursor::after-move": fun(Core.Document)
---         after the Core.Cursor is moved via Core.Viewport:move_cursor
---
---     - "document::created" | "document::destroyed": fun(Core.Document)
---         after a Core.Document was created or destroyed
---     - "document::before-file-load" | "document::after-file-load": fun(Core.Document)
---         before or after a Core.Document read a file from disk
---     - "document::file-type": fun(Core.Document)
---         after a Core.Document was created and the backing filepath has no file extension
---     - "document::file-type-XYZ": fun(Core.Document)
---         after a Core.Document was created and the backing filepath has a file extension. XYZ is replaced with the
---             file extension *excluding* the dot.
---     - "document::loaded" | "document::unloaded": fun(Core.Document)
---         after a Core.Document is loaded from the background or unloaded into the background
---     - "document::focus" | "document::unfocus": fun(Core.Document)
---         after a Core.Document is focused in a Core.Viewport. If the same Core.Document is present in multiple
---             Core.Viewports, switching between those will *not* emit this event.
---     - "document::before-save" | "document::after-save": fun(Core.Document)
---         before or after a Core.Document is saved using Core.Document:save
---
---     - "mini_buffer::created": fun()
---         after the Mini Buffer was created. It does *not* emit document:: and viewport:: events for its internal
---             Core.Documents and Core.Viewports
---
---     - "viewport::created" | "viewport::destroyed": fun(Core.Viewport)
---         after a Core.Viewport was created or destroyed
---     - "viewport::focus" | "viewport::unfocus": fun(Core.Viewport)
---         after a Core.Viewport is focus or unfocused
---     - "viewport::resized": fun(Core.Viewport)
---         after a Core.Viewport was resized
--- @param event string The name of the hook.
--- @param callback function The function to call.
function Hooks.add(event, callback)
    if not Hooks.registry[event] then
        Hooks.registry[event] = {}
    end

    table.insert(Hooks.registry[event], callback)
end

--- Runs all callbacks for a specific hook.
--- @param event string
--- @param ... any Arguments passed to the callback.
function Hooks.run(event, ...)
    local callbacks = Hooks.registry[event]

    if not callbacks then return end

    for _, callback in ipairs(callbacks) do
        local ok, err = xpcall(callback, debug.traceback, ...)
        if not ok then
            Cini:set_status_message("Failed to run hook for '" .. event .. "':\n" .. tostring(err),
                "error_message", 0, true)
        end
    end
end

--- Runs all callbacks for a specific hook and aggregates boolean results.
--- Returns true if all callbacks return true (or nil), false otherwise.
--- @param event string
--- @param ... any Arguments passed to the callback.
--- @return boolean
function Hooks.run_boolean(event, ...)
    local callbacks = Hooks.registry[event]

    if not callbacks then return true end

    local ret = true
    for _, callback in ipairs(callbacks) do
        local ok, res = xpcall(callback, debug.traceback, ...)
        if not ok then
            Cini:set_status_message("Failed to run hook for '" .. event .. "':\n" .. tostring(res),
                "error_message", 0, true)
        else
            ret = ret and res
        end
    end

    return ret
end

return Hooks
