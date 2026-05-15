--- @meta

--- An asyncronously running process.
--- @class Core.AsyncProcess
--- @field command string The running command.
--- @field args string[] The arguments to the command.
--- @field doc Core.Document The Document the process is writing to.
Core.AsyncProcess = {}

--- Starts a process.
--- @param color boolean Enable or disable (ANSI) color output.
--- @return boolean
function Core.AsyncProcess:spawn(color) end

--- Kills a process.
function Core.AsyncProcess:kill() end
