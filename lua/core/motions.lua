--- @class Core.Motions
local Motions = {}

--- @class Core.Motion
--- @field sequence string
--- @field run fun(cur: Core.Cursor, view: Core.DocumentView, n: integer)
local Motion = {}

--- Global motion registry.
--- @type table<string, Core.Motion>
Motions.motions = {}

function Motions.init()
    Core.Motions = Motions
end

--- Registers a new motion
--- @param name string
--- @param motion Core.Motion
function Motions.register_motion(name, motion)
    Motions.motions[name] = motion

    Core.Hooks.run("motion::registered", name, motion)
end

--- Retrieves a motion.
--- @param name string
--- @return Core.Motion?
function Motions.get_motion(name)
    return Motions.motions[name]
end

--- Applies an action on a motion. The action must return how many characters got added/removed by it.
--- @param motion Core.Motion
--- @param arg integer
--- @param action fun(view: Core.DocumentView, start: integer, stop: integer): integer
function Motions.apply(motion, arg, action)
    local view = Cini.workspace.viewport.view
    local start_pos = view.cur:point(view)
    local start = start_pos

    -- Do this manual to avoid emitting cursor move events.
    motion.run(view.cur, view, arg)
    local stop_pos = view.cur:point(view)
    local stop = stop_pos

    if stop < start then start, stop = stop, start end
    local offset = action(view, start, stop)

    -- Do this manual to avoid emitting cursor move events. Position cursor at expected location. If the action works
    -- backwards compensate for removed/added characters.
    if stop_pos < start_pos then
        view.cur:move_to(view, start_pos + offset)
    else
        view.cur:move_to(view, start_pos)
    end
end

return Motions
