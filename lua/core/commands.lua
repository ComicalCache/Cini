--- @class Core.Commands
local Commands = {}

--- @class Core.Command
--- @field run function The function to run on command execution.
--- @field metadata table Free-form metadata.
local Command = {}

--- Global command registry.
--- @type table<string, Core.Command>
Commands.registry = {}

function Commands.init()
    Core.Commands = Commands
end

--- Registers a command.
--- @param name string Command name.
--- @param cmd Core.Command
function Commands.register(name, cmd)
    Commands.registry[name] = cmd
end

--- Retrieves a command.
--- @return Core.Command?
function Commands.get(name)
    return Commands.registry[name]
end

return Commands
