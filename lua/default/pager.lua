local Pager = {}

function Pager.setup()
    -- Modes.
    Core.Modes.register_mode({
        name = "pager",
        cursor_style = Core.CursorStyle.SteadyBlock,
        mode_line_layout = {
            { run = function(_) return { { text = "PAGER" } } end },
            "minor_mode_indicators",
            "pending_keys",
            "spacer",
            "cursor_row",
        }
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
    Core.Hooks.add("document::set-major-mode", 50, function(doc, mode)
        --- @cast doc Core.Document
        --- @cast mode string

        if mode ~= "pager" then return end

        for _, view in ipairs(doc:views()) do
            view.gutter = false

            view.properties["ws"] = nil
            view.properties["nl"] = nil
            view.properties["tab"] = nil
        end
    end)

    Core.Hooks.add("document_view::created", 50, function(view)
        --- @cast view Core.DocumentView

        local mode = Core.Modes.get_major_mode(view.doc)
        if mode and mode.name == "pager" then
            view.gutter = false

            view.properties["ws"] = nil
            view.properties["nl"] = nil
            view.properties["tab"] = nil
        end
    end)
end

function Pager.init() end

return Pager
