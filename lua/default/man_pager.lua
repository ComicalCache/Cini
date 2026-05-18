local ManPager = {}

--- @class ManPager.Style
--- @field start integer
--- @field stop integer
--- @field face Core.Face

function ManPager.setup()
    -- Modes.
    Core.Modes.register_mode({
        name = "man_pager",
        cursor_style = Core.CursorStyle.SteadyBlock,
        mode_line_layout = {
            { run = function(_) return { { text = "MAN" } } end },
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
    Core.Hooks.add("document::set-major-mode", "man_pager.setup", 50, function(doc, mode)
        if mode ~= "man_pager" then return end

        -- Remove properties that might have been put there by the AnsiTextStream parser.
        doc:clear_text_properties()

        for _, view in ipairs(doc:views()) do view.gutter = false end

        local raw = doc:slice(0, doc.size)
        if not raw:find('\x08') then return end
        local formatted = {}

        --- @type ManPager.Style[]
        local styles = {}

        local curr_byte = 0
        local idx = 1
        while idx <= #raw do
            local len = Core.Utf8.len(raw:sub(idx, idx))
            local ch = raw:sub(idx, idx + len - 1)

            -- Check for backspace character.
            if idx + len <= #raw and raw:sub(idx + len, idx + len) == '\x08' then
                local draw_ch_len = Core.Utf8.len(raw:sub(idx + len + 1, idx + len + 1))
                local draw_ch = raw:sub(idx + len + 1, idx + len + draw_ch_len)

                if ch == '_' then
                    table.insert(styles,
                        { start = curr_byte, stop = curr_byte + draw_ch_len, face = Core.Face({ underline = true }) })
                elseif ch == draw_ch then
                    table.insert(styles,
                        { start = curr_byte, stop = curr_byte + draw_ch_len, face = Core.Face({ bold = true }) })
                end

                table.insert(formatted, draw_ch)
                curr_byte = curr_byte + draw_ch_len
                idx = idx + len + 1 + draw_ch_len
            else
                table.insert(formatted, ch)
                curr_byte = curr_byte + len
                idx = idx + len
            end
        end

        doc:clear()
        doc:insert(0, table.concat(formatted))
        doc.modified = false

        for _, style in ipairs(styles) do
            doc:add_text_property(style.start, style.stop, "face", style.face)
        end
    end)

    Core.Hooks.add("document_view::created", "man_pager.synchronize_new_document_view", 50, function(view)
        local mode = Core.Modes.get_major_mode(view.doc)
        if mode and mode.name == "man_pager" then view.gutter = false end
    end)
end

function ManPager.init() end

return ManPager
