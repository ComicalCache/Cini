local Pager = {}

--- @class Pager.Style
--- @field start integer
--- @field stop integer
--- @field face Core.Face

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

        local raw = doc:slice(0, doc.size)
        if not raw:find('\x08') then return end
        local formatted = {}

        --- @type table<integer, Pager.Style>
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
end

function Pager.init() end

return Pager
