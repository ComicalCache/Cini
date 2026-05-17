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
        },
        metadata = {
            read_only = true
        }
    })

    -- Hooks.
    Core.Hooks.add("document::set-major-mode", 50, function(doc, mode)
        --- @cast doc Core.Document
        --- @cast mode string

        if mode ~= "pager" then return end

        for _, view in ipairs(doc:views()) do view.gutter = false end
    end)

    Core.Hooks.add("document_view::created", 50, function(view)
        --- @cast view Core.DocumentView

        local mode = Core.Modes.get_major_mode(view.doc)
        if mode and mode.name == "pager" then view.gutter = false end
    end)
end

function Pager.init() end

return Pager
