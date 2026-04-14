local ModeLineDefaults = {}

function ModeLineDefaults.setup()
    Core.ModeLine.register_component("mode_names", {
        run = function(viewport)
            local view = viewport.view
            local major_mode = Core.Modes.get_major_mode(view.doc)
            local minor_mode_override = Core.Modes.get_minor_mode_override(view)

            local text = ""
            if major_mode and minor_mode_override then
                text = "[" .. major_mode.name:upper() .. " | " .. minor_mode_override.name:upper() .. "]"
            elseif major_mode then
                text = "[" .. major_mode.name:upper() .. "]"
            elseif minor_mode_override then
                text = "[" .. minor_mode_override.name:upper() .. "]"
            else
                text = "[No Mode]"
            end
            return { { text = text } }
        end
    })

    Core.ModeLine.register_component("minor_mode_indicators", {
        run = function(viewport)
            local view = viewport.view
            local minor_modes = Core.Modes.get_minor_modes(view)

            local ret = {}

            for idx = #minor_modes, 1, -1 do
                local mode = minor_modes[idx]
                local indicator = Core.ModeLine.indicators[mode.name]

                if indicator then
                    local segments = indicator.run(viewport)
                    if segments then
                        for _, seg in ipairs(segments) do table.insert(ret, seg) end
                    end
                end
            end

            return ret
        end
    })

    Core.ModeLine.register_component("filename", {
        run = function(viewport)
            local view = viewport.view
            local name = view.doc.properties["name"] or ((view.doc.path or ""):match("([^/]+)$") or "Scratchpad")

            return { { text = "[" .. name .. "]" } }
        end
    })

    Core.ModeLine.register_component("file_info", {
        run = function(viewport)
            local view = viewport.view
            local name = view.doc.properties["name"] or ((view.doc.path or ""):match("([^/]+)$") or "Scratchpad")

            return { { text = ("[%s%s | %dB]"):format(name, (view.doc.modified and " *" or ""), view.doc.size) } }
        end
    })

    Core.ModeLine.register_component("pending_keys", {
        run = function(_)
            local pending_keys = Core.Keybinds.pending_keys

            if pending_keys and #pending_keys > 0 then
                return { {
                    text = table.concat(pending_keys, " "),
                    face = Core.Face({ fg = Core.Rgb(97, 175, 239) })
                } }
            end
            return {}
        end
    })

    Core.ModeLine.register_component("cursor_row", {
        run = function(viewport)
            local view = viewport.view
            local max_row = view.doc:position_from_byte(view.doc.size).row + 1

            return { { text = ("%d/%d"):format(view.cur.row + 1, max_row) } }
        end
    })

    Core.ModeLine.register_component("cursor_pos", {
        run = function(viewport)
            local view = viewport.view
            local max_row = view.doc:position_from_byte(view.doc.size).row + 1
            return { { text = ("%d:%d/%d"):format(view.cur.row + 1, view.cur.col + 1, max_row) } }
        end
    })

    -- Hooks.
    Core.Hooks.add("document_view::created", 10, function(view)
        --- @cast view Core.DocumentView
        view:set_mode_line(function(vp)
            local major_mode = Core.Modes.get_major_mode(vp.view.doc)

            if major_mode and major_mode.mode_line_layout then
                return Core.ModeLine.render(vp, major_mode.mode_line_layout)
            else
                return Core.ModeLine.render(vp, Core.ModeLine.default_layout)
            end
        end)
    end)

    -- Defaults.
    Core.ModeLine.default_layout = {
        "mode_names",
        "minor_mode_indicators",
        "file_info",
        "pending_keys",
        "spacer",
        "cursor_pos",
    }
end

function ModeLineDefaults.init() end

return ModeLineDefaults
