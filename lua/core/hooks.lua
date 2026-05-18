--- @class Core.Hooks
local Hooks = {}

--- @class Core.Hook
--- @field id string The id of the hook.
--- @field priority number The priority of the hook.
--- @field callback function The function to run on the hook event.

--- @type table<string, Core.Hook[]>
Hooks.registry = {}

function Hooks.init()
    Core.Hooks = Hooks
end

--- Registers a callback for a specific hook.
---
--- @overload fun(event: "cini::startup", id: string, priority: number, callback: fun())
--- @overload fun(event: "cini::shutdown", id: string, priority: number, callback: fun())
--- @overload fun(event: "command::before-execute", id: string, priority: number, callback: fun(name: string, cmd: Core.Command): boolean)
--- @overload fun(event: "cursor::before-move", id: string, priority: number, callback: fun(view: Core.DocumentView, target: integer): boolean)
--- @overload fun(event: "cursor::after-move", id: string, priority: number, callback: fun(view: Core.DocumentView, pos: integer))
--- @overload fun(event: "document::created", id: string, priority: number, callback: fun(doc: Core.Document))
--- @overload fun(event: "document::destroyed", id: string, priority: number, callback: fun(doc: Core.Document))
--- @overload fun(event: "document::before-file-load", id: string, priority: number, callback: fun(doc: Core.Document))
--- @overload fun(event: "document::after-file-load", id: string, priority: number, callback: fun(doc: Core.Document))
--- @overload fun(event: "document::file-type", id: string, priority: number, callback: fun(doc: Core.Document, type: string?))
--- @overload fun(event: "document::loaded", id: string, priority: number, callback: fun(doc: Core.Document))
--- @overload fun(event: "document::unloaded", id: string, priority: number, callback: fun(doc: Core.Document))
--- @overload fun(event: "document::before-insert", id: string, priority: number, callback: fun(doc: Core.Document, start: integer, len: integer))
--- @overload fun(event: "document::after-insert", id: string, priority: number, callback: fun(doc: Core.Document, start: integer, len: integer))
--- @overload fun(event: "document::before-remove", id: string, priority: number, callback: fun(doc: Core.Document, start: integer, len: integer))
--- @overload fun(event: "document::after-remove", id: string, priority: number, callback: fun(doc: Core.Document, start: integer, len: integer))
--- @overload fun(event: "document::before-clear", id: string, priority: number, callback: fun(doc: Core.Document))
--- @overload fun(event: "document::after-clear", id: string, priority: number, callback: fun(doc: Core.Document))
--- @overload fun(event: "document::before-save", id: string, priority: number, callback: fun(doc: Core.Document))
--- @overload fun(event: "document::after-save", id: string, priority: number, callback: fun(doc: Core.Document))
--- @overload fun(event: "document::set-major-mode", id: string, priority: number, callback: fun(doc: Core.Document, name: string))
--- @overload fun(event: "document::unset-major-mode", id: string, priority: number, callback: fun(doc: Core.Document, name: string))
--- @overload fun(event: "document_view::created", id: string, priority: number, callback: fun(view: Core.DocumentView))
--- @overload fun(event: "document_view::destroyed", id: string, priority: number, callback: fun(view: Core.DocumentView))
--- @overload fun(event: "document_view::loaded", id: string, priority: number, callback: fun(view: Core.DocumentView))
--- @overload fun(event: "document_view::unloaded", id: string, priority: number, callback: fun(view: Core.DocumentView))
--- @overload fun(event: "document_view::focus", id: string, priority: number, callback: fun(view: Core.DocumentView))
--- @overload fun(event: "document_view::unfocus", id: string, priority: number, callback: fun(view: Core.DocumentView))
--- @overload fun(event: "mini_buffer::created", id: string, priority: number, callback: fun())
--- @overload fun(event: "motion::registered", id: string, priority: number, callback: fun(name: string, motion: Core.Motion))
--- @overload fun(event: "process::spawned", id: string, priority: number, callback: fun(process: Core.AsyncProcess))
--- @overload fun(event: "process::exited", id: string, priority: number, callback: fun(process: Core.AsyncProcess, code: integer))
--- @overload fun(event: "viewport::created", id: string, priority: number, callback: fun(viewport: Core.Viewport))
--- @overload fun(event: "viewport::destroyed", id: string, priority: number, callback: fun(viewport: Core.Viewport))
--- @overload fun(event: "viewport::focus", id: string, priority: number, callback: fun(viewport: Core.Viewport))
--- @overload fun(event: "viewport::unfocus", id: string, priority: number, callback: fun(viewport: Core.Viewport))
--- @overload fun(event: "viewport::resized", id: string, priority: number, callback: fun(viewport: Core.Viewport))
---
--- @param event string The name of the hook.
--- @param id string The id of the hook.
--- @param priority number The priority of the hook (lower runs first).
--- @param callback function The function to call.
function Hooks.add(event, id, priority, callback)
    if not Hooks.registry[event] then Hooks.registry[event] = {} end

    table.insert(Hooks.registry[event], { id = id, priority = priority, callback = callback, })
    table.sort(Hooks.registry[event], function(a, b) return a.priority < b.priority end)
end

--- Removes a hook.
--- @param event string The name of the hook event.
--- @param id string The id of the hook.
--- @return boolean true if the hook was removed, false otherwise.
function Hooks.remove(event, id)
    if not id then return false end

    local entries = Hooks.registry[event]
    if not entries then return false end

    for idx = #entries, 1, -1 do
        if entries[idx].id == id then
            table.remove(entries, idx)

            return true
        end
    end

    return false
end

--- Runs all callbacks for a specific hook.
--- @param event string
--- @param ... any Arguments passed to the callback.
function Hooks.run(event, ...)
    local entries = Hooks.registry[event]

    if not entries then return end

    for _, entry in ipairs(entries) do
        local ok, err = xpcall(entry.callback, debug.traceback, ...)
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
    local entries = Hooks.registry[event]

    if not entries then return true end

    local ret = true
    for _, entry in ipairs(entries) do
        local ok, res = xpcall(entry.callback, debug.traceback, ...)
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
