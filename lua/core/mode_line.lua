--- @class Core.ModeLine
local ModeLine = {}

--- @class Core.ModeLine.Component
--- @field run fun(viewport: Core.Viewport): table? (List of) segments. A segment is either
---                                                 { text = "...", face = "..." } or { spacer = true }.

--- Global mode line component registry.
--- @type table<string, Core.ModeLine.Component>
ModeLine.components = {}

--- Global minor mode indicator registry.
--- @type table<string, Core.ModeLine.Component>
ModeLine.indicators = {}

--- @type (string|Core.ModeLine.Component)[]
ModeLine.default_layout = {}

function ModeLine.init()
    Core.ModeLine = ModeLine
end

--- Registers a mode line component.
--- @param name string
--- @param component Core.ModeLine.Component
function ModeLine.register_component(name, component)
    ModeLine.components[name] = component
end

--- Registers a minor mode indicator.
--- @param name string
--- @param indicator Core.ModeLine.Component
function ModeLine.register_indicator(name, indicator)
    ModeLine.indicators[name] = indicator
end

--- Retrieves a component by name.
--- @param name string
--- @return Core.ModeLine.Component?
function ModeLine.get_component(name)
    return ModeLine.components[name]
end

--- Renders a layout array.
--- @param viewport Core.Viewport
--- @param layout (string|Core.ModeLine.Component)[]
--- @return table
function ModeLine.render(viewport, layout)
    local ret = {}

    -- Leading space.
    table.insert(ret, { text = " " })

    local need_space = false
    for _, item in ipairs(layout) do
        if type(item) == "string" then
            if item == "spacer" then
                table.insert(ret, { spacer = true })
                need_space = false
            else
                local comp = ModeLine.get_component(item)
                if comp then
                    local segments = comp.run(viewport)

                    if segments and #segments > 0 then
                        if need_space then table.insert(ret, { text = " " }) end

                        for _, seg in ipairs(segments) do table.insert(ret, seg) end
                        need_space = true
                    end
                end
            end
        else
            local segments = item.run(viewport)
            if segments and #segments > 0 then
                if need_space then table.insert(ret, { text = " " }) end

                for _, seg in ipairs(segments) do table.insert(ret, seg) end
                need_space = true
            end
        end
    end

    -- Trailing space.
    table.insert(ret, { text = " " })

    return ret
end

return ModeLine
