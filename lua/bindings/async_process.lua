--- @meta

--- An asyncronously running process.
--- @class Core.AsyncProcess
--- @field command string The running command.
--- @field args table<integer, string> The arguments to the command.
--- @field doc Core.Document The Document the process is writing to.
Core.AsyncProcess = {}

--- Starts a process.
--- @return boolean
function Core.AsyncProcess:spawn() end

--- Kills a process.
function Core.AsyncProcess:kill() end
