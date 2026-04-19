local Command = {}

function Command.setup()
    -- Hooks.
    Core.Hooks.add("document::after-insert", 50, function(doc, pos, len)
        --- @cast doc Core.Document
        --- @cast pos integer
        --- @cast len integer

        if not doc.properties.process_attached then return end

        -- Move all cursors to the end of the Document after insertion if already at the end.
        for _, view in ipairs(doc:views()) do
            if view.cur:point(view) == pos then view:move_cursor(function(c, v) c:move_to(v, pos + len) end, 0) end
        end
    end)

    Core.Hooks.add("process::exited", 10, function(process, code)
        --- @cast process Core.AsyncProcess
        --- @cast code integer

        Cini:set_status_message(("Process '%s' exited with code %d"):format(process.command, code), "info_message", 0,
            false)
    end)

    -- Commands.
    Core.Commands.register("command.run", {
        metadata = { modifies = true },
        run = function()
            Core.Prompt.run("Command: ", "", function(input)
                if not input or input:match("^%s*$") then return end

                local args = {}
                for word in input:gmatch("%S+") do
                    table.insert(args, word)
                end

                local cmd = table.remove(args, 1)
                if not cmd then return end

                local view = Cini.workspace.viewport.view
                if not Cini:create_process(cmd, args, view.doc, view.cur:point(view)):spawn() then
                    Cini:set_status_message(("Failed to spawn process '%s'"):format(cmd), "error_message", 0, false)
                end
            end)
        end
    })

    -- Keybinds.
    Core.Keybinds.bind("global", "<C-p>", "command.run")
end

function Command.init() end

return Command
