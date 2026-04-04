--- @class Core.Motions
local Motions = {}

--- @class Core.Motion
--- @field sequence string
--- @field run fun(cursor: Core.Cursor, doc: Core.Document, n: integer)
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
--- @param action fun(doc: Core.Document, start: integer, stop: integer): integer
function Motions.apply(motion, arg, action)
    local viewport = Cini.workspace.viewport
    local start_pos = viewport.cursor:point(viewport.doc)
    local start = start_pos

    -- Do this manual to avoid emitting cursor move events.
    motion.run(viewport.cursor, viewport.doc, arg)
    local stop_pos = viewport.cursor:point(viewport.doc)
    local stop = stop_pos

    if stop < start then start, stop = stop, start end
    local offset = action(viewport.doc, start, stop)

    -- Position cursor at expected location. If the action works backwards compensate for removed/added characters.
    if stop_pos < start_pos then
        viewport.cursor:move_to(viewport.doc, start_pos + offset)
    else
        viewport.cursor:move_to(viewport.doc, start_pos)
    end
end

return Motions
