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
    Core.Hooks.add("document::set-major-mode", "pager.setup", 50, function(doc, mode)
        if mode ~= "pager" then return end

        for _, view in ipairs(doc:views()) do view.gutter = false end
    end)

    Core.Hooks.add("document_view::created", "pager.synchronize_new_document_view", 50, function(view)
        local mode = Core.Modes.get_major_mode(view.doc)
        if mode and mode.name == "pager" then view.gutter = false end
    end)
end

function Pager.init() end

return Pager
