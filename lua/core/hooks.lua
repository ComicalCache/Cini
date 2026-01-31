--- @class Core.Hooks
local Hooks = {}

--- @type table<string, function[]>
Hooks.registry = {}

function Hooks.init()
    Core.Hooks = Hooks
end

--- Registers a callback for a specific hook.
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

    if callbacks then
        for _, callback in ipairs(callbacks) do
            local ok, err = xpcall(callback, debug.traceback, ...)
            if not ok then
                State.editor:set_status_message("Failed to run hook for '" .. event .. "':\n" .. tostring(err),
                    "error_message", 5000, true)
            end
        end
    end
end

return Hooks
