local Pager = {}

function Pager.setup()
    -- Modes.
    Core.Modes.register_mode({
        name = "pager",
        cursor_style = Core.CursorStyle.SteadyBlock,
        mode_line_layout = {
            { callback = function(_) return { { text = "PAGER" } } end },
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
    Core.Hooks.add("document::set-major-mode", {
            id = "pager.setup",
            priority = 50,
            metadata = { description = "Sets up all DocumentViews of a pager Document." }
        },
        function(doc, mode)
            if mode ~= "pager" then return end

            for _, view in ipairs(doc:views()) do view.gutter = false end
        end)

    Core.Hooks.add("document_view::created", {
            id = "pager.synchronize_new_document_view",
            priority = 50,
            metadata = { description = "Sets up new DocumentViews for a pager Document." }
        },
        function(view)
            local mode = Core.Modes.get_major_mode(view.doc)
            if mode and mode.name == "pager" then view.gutter = false end
        end)
end

function Pager.init() end

return Pager
