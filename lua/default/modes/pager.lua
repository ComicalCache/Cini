local Pager = {}

function Pager.setup()
    -- Modes.
    Core.Modes.register_mode({
        name = "pager",
        cursor_style = Core.CursorStyle.SteadyBlock,
        mode_line = function(viewport)
            local view = viewport.view
            local doc = view.doc

            local filename = ((view.doc.path or ""):match("([^/]+)$") or "Scratchpad")
            local curr_line = view.cur.row + 1
            local total_lines = doc:position_from_byte(doc.size).row + 1

            return { { text = (" PAGER [%s | %d/%d]"):format(filename, curr_line, total_lines) } }
        end
    })

    -- Hooks.
    Core.Hooks.add("command::before-execute", 50, function(_, cmd)
        --- @cast cmd Core.Command

        if Cini.workspace.is_mini_buffer then return true end

        local view = Cini.workspace.viewport.view
        local mode = Core.Modes.get_major_mode(view.doc)

        local legal = (cmd.metadata and cmd.metadata.modifies)
        return not (legal and mode and mode.name == "pager")
    end)
end

function Pager.init() end

return Pager
