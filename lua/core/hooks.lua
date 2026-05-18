--- @class Core.Hooks
local Hooks = {}

--- @class Core.HookConfig
--- @field id string The id of the hook.
--- @field priority number The priority of the hook.
--- @field metadata? table Free-form metadata.

--- @class Core.Hook : Core.HookConfig
--- @field callback function The function to run on the hook event.

--- @type table<string, Core.Hook[]>
Hooks.registry = {}

function Hooks.init()
    Core.Hooks = Hooks
end

--- Registers a callback for a specific hook.
---
--- @overload fun(event: "cini::startup", config: Core.HookConfig, callback: fun())
--- @overload fun(event: "cini::shutdown", config: Core.HookConfig, callback: fun())
--- @overload fun(event: "command::before-execute", config: Core.HookConfig, callback: fun(name: string, cmd: Core.Command): boolean)
--- @overload fun(event: "cursor::before-move", config: Core.HookConfig, callback: fun(view: Core.DocumentView, target: integer): boolean)
--- @overload fun(event: "cursor::after-move", config: Core.HookConfig, callback: fun(view: Core.DocumentView, pos: integer))
--- @overload fun(event: "document::created", config: Core.HookConfig, callback: fun(doc: Core.Document))
--- @overload fun(event: "document::destroyed", config: Core.HookConfig, callback: fun(doc: Core.Document))
--- @overload fun(event: "document::before-file-load", config: Core.HookConfig, callback: fun(doc: Core.Document))
--- @overload fun(event: "document::after-file-load", config: Core.HookConfig, callback: fun(doc: Core.Document))
--- @overload fun(event: "document::file-type", config: Core.HookConfig, callback: fun(doc: Core.Document, type: string?))
--- @overload fun(event: "document::loaded", config: Core.HookConfig, callback: fun(doc: Core.Document))
--- @overload fun(event: "document::unloaded", config: Core.HookConfig, callback: fun(doc: Core.Document))
--- @overload fun(event: "document::before-insert", config: Core.HookConfig, callback: fun(doc: Core.Document, start: integer, len: integer))
--- @overload fun(event: "document::after-insert", config: Core.HookConfig, callback: fun(doc: Core.Document, start: integer, len: integer))
--- @overload fun(event: "document::before-remove", config: Core.HookConfig, callback: fun(doc: Core.Document, start: integer, len: integer))
--- @overload fun(event: "document::after-remove", config: Core.HookConfig, callback: fun(doc: Core.Document, start: integer, len: integer))
--- @overload fun(event: "document::before-clear", config: Core.HookConfig, callback: fun(doc: Core.Document))
--- @overload fun(event: "document::after-clear", config: Core.HookConfig, callback: fun(doc: Core.Document))
--- @overload fun(event: "document::before-save", config: Core.HookConfig, callback: fun(doc: Core.Document))
--- @overload fun(event: "document::after-save", config: Core.HookConfig, callback: fun(doc: Core.Document))
--- @overload fun(event: "document::set-major-mode", config: Core.HookConfig, callback: fun(doc: Core.Document, name: string))
--- @overload fun(event: "document::unset-major-mode", config: Core.HookConfig, callback: fun(doc: Core.Document, name: string))
--- @overload fun(event: "document_view::created", config: Core.HookConfig, callback: fun(view: Core.DocumentView))
--- @overload fun(event: "document_view::destroyed", config: Core.HookConfig, callback: fun(view: Core.DocumentView))
--- @overload fun(event: "document_view::loaded", config: Core.HookConfig, callback: fun(view: Core.DocumentView))
--- @overload fun(event: "document_view::unloaded", config: Core.HookConfig, callback: fun(view: Core.DocumentView))
--- @overload fun(event: "document_view::focus", config: Core.HookConfig, callback: fun(view: Core.DocumentView))
--- @overload fun(event: "document_view::unfocus", config: Core.HookConfig, callback: fun(view: Core.DocumentView))
--- @overload fun(event: "mini_buffer::created", config: Core.HookConfig, callback: fun())
--- @overload fun(event: "motion::registered", config: Core.HookConfig, callback: fun(name: string, motion: Core.Motion))
--- @overload fun(event: "process::spawned", config: Core.HookConfig, callback: fun(process: Core.AsyncProcess))
--- @overload fun(event: "process::exited", config: Core.HookConfig, callback: fun(process: Core.AsyncProcess, code: integer))
--- @overload fun(event: "viewport::created", config: Core.HookConfig, callback: fun(viewport: Core.Viewport))
--- @overload fun(event: "viewport::destroyed", config: Core.HookConfig, callback: fun(viewport: Core.Viewport))
--- @overload fun(event: "viewport::focus", config: Core.HookConfig, callback: fun(viewport: Core.Viewport))
--- @overload fun(event: "viewport::unfocus", config: Core.HookConfig, callback: fun(viewport: Core.Viewport))
--- @overload fun(event: "viewport::resized", config: Core.HookConfig, callback: fun(viewport: Core.Viewport))
---
--- @param event string The name of the hook.
--- @param config Core.HookConfig The configuration table.
--- @param callback function The function to run.
function Hooks.add(event, config, callback)
    if not Hooks.registry[event] then Hooks.registry[event] = {} end

    local hook = {
        id = config.id,
        priority = config.priority,
        metadata = config.metadata,
        callback = callback
    }

    table.insert(Hooks.registry[event], hook)
    table.sort(Hooks.registry[event], function(a, b)
        if a.priority == b.priority then return a.id < b.id end
        return a.priority < b.priority
    end)
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
