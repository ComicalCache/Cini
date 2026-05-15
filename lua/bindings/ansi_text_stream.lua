--- @meta

--- A stateful ANSI state machine parser.
--- @class Core.AnsiTextStream
Core.AnsiTextStream = {}


--- @param doc Core.Document Document into which the parsed ANSI text gets parsed.
--- @return Core.AnsiTextStream
function Core.AnsiTextStream(doc) end

--- Parses an ANSI text stream into the Document at a specified position.
--- @param text string
--- @param pos integer
--- @return integer The pos after inserting.
function Core.AnsiTextStream:parse(text, pos) end

--- Flushes the remaining data of the parser into the Document at a specified position.
--- @param pos integer
--- @return integer The pos after inserting.
function Core.AnsiTextStream:flush(pos) end
