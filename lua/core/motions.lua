--- @class Core.Motions
local Motions = {}

--- Global motion registry.
--- @type table<string, fun(cursor: Core.Cursor, doc: Core.Document, n: integer)>
Motions.motions = {}

function Motions.init()
    Core.Motions = Motions
end

--- Registers a new motion
--- @param sequence string
--- @param motion fun(cursor: Core.Cursor, doc: Core.Document, n: integer)
function Motions.register_motion(sequence, motion)
    Motions.motions[sequence] = motion
end

--- Retrieves a motion.
--- @param sequence string
--- @return fun(cursor: Core.Cursor, doc: Core.Document, n: integer)?
function Motions.get_motion(sequence)
    return Motions.motions[sequence]
end

--- Applies an action on a motion
--- @param motion fun(cursor: Core.Cursor, doc: Core.Document, n: integer)
--- @param arg integer
--- @param action fun(doc: Core.Document, start: integer, stop: integer)
function Motions.apply(motion, arg, action)
    local viewport = Cini.workspace.viewport
    local start = viewport.doc.point

    -- Do this manual to avoid emitting cursor move events.
    motion(viewport.cursor, viewport.doc, arg)
    local stop = viewport.cursor:point(viewport.doc)

    if stop < start then start, stop = stop, start end
    viewport.cursor:move_to(viewport.doc, start)

    action(viewport.doc, start, stop)
end

return Motions
