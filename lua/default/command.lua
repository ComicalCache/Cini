local Command = {}

function Command.setup()
    -- Hooks.
    Core.Hooks.add("document::after-insert", {
            id = "command.move_cursor",
            priority = 50,
            metadata = { description = "Move the cursors of all DocumentViews after inserting process output." }
        },
        function(doc, pos, len)
            if not doc.properties.process_attached then return end

            -- Move all cursors to the end of the Document after insertion if already at the end.
            for _, view in ipairs(doc:views()) do
                if view.cur:point(view) == pos then view:move_cursor(function(c, v) c:move_to(v, pos + len) end, 0) end
            end
        end)

    Core.Hooks.add("process::exited", {
            id = "command.exit",
            priority = 10,
            metadata = { description = "Prints information about a process' exit code." }
        },
        function(process, code)
            process.doc.properties.process_attached = nil
            Cini:set_status_message(("Process '%s' exited with code %d"):format(process.command, code), "info_message",
                3000, false)
        end)

    -- Commands.
    Core.Commands.register("command.run", {
        metadata = {
            modifies = true,
            synopsis = "Runs a process or command",
            description =
            "Execute a process or command, capture its stdout and stderr output and enter it in the current buffer."
        },
        callback = function()
            Core.Prompt.run("Command: ", "", function(input)
                if not input or input:match("^%s*$") then return end

                local args = {}
                for word in input:gmatch("%S+") do
                    table.insert(args, word)
                end

                local cmd = table.remove(args, 1)
                if not cmd then return end

                local view = Cini.workspace.viewport.view
                local parser = Core.AnsiTextStream(view.doc)
                local pos = view.cur:point(view)

                --- @type fun(process: Core.AsyncProcess, len: integer, data: string?)
                local callback = function(process, len, data)
                    if len > 0 then
                        -- data is guaranteed to not be nil if len > 0.
                        --- @cast data string
                        pos = parser:parse(data, pos)
                    elseif len < 0 then
                        -- The process has terminated.
                        parser:flush(pos)
                    end

                    Cini:request_render()
                end
                if not Cini:create_process(cmd, args, view.doc, callback):spawn(true) then
                    view.doc.properties.process_attached = true

                    Cini:set_status_message(("Failed to spawn process '%s'"):format(cmd), "error_message", 3000, false)
                end
            end)
        end
    })

    -- Keybinds.
    Core.Keybinds.bind("global", "<C-p>", "command.run")
end

function Command.init() end

return Command
