local Git = {}

function Git.setup()
    -- Faces.
    Core.Faces.register_face("git.green", Core.Face({ fg = Core.Rgb(84, 212, 84) }))
    Core.Faces.register_face("git.red", Core.Face({ fg = Core.Rgb(212, 84, 84) }))
    Core.Faces.register_face("git.cyan", Core.Face({ fg = Core.Rgb(84, 212, 212) }))
    Core.Faces.register_face("git.yellow", Core.Face({ fg = Core.Rgb(212, 212, 84) }))

    -- Modes.
    Core.Modes.register_mode({
        name = "git",
        cursor_style = Core.CursorStyle.SteadyBlock,
        mode_line_layout = {
            { run = function(_) return { { text = "GIT" } } end },
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
        return not (legal and mode and mode.name == "git")
    end)

    Core.Hooks.add("document::set-major-mode", 50, function(doc, mode)
        --- @cast doc Core.Document
        --- @cast mode string

        if mode ~= "git" then return end

        for _, view in ipairs(doc:views()) do view.gutter = false end

        for i = 0, doc.lines - 1 do
            local line = doc:line(i)
            local start = doc:line_begin_byte(i)
            local stop = doc:line_end_byte(i)

            if line:sub(1, 1) == "+" and line:sub(1, 3) ~= "+++" then
                doc:add_text_property(start, stop, "face", "git.green")
            elseif line:sub(1, 1) == "-" and line:sub(1, 3) ~= "---" then
                doc:add_text_property(start, stop, "face", "git.red")
            elseif line:sub(1, 2) == "@@" then
                local new_stop = line:sub(3):find("@@")
                if new_stop then
                    doc:add_text_property(start, start + new_stop + 3, "face", "git.cyan")
                end
            elseif line:sub(1, 6) == "commit" then
                doc:add_text_property(start, stop, "face", "git.yellow")
            end
        end
    end)

    Core.Hooks.add("document_view::created", 50, function(view)
        --- @cast view Core.DocumentView

        local mode = Core.Modes.get_major_mode(view.doc)

        if mode and mode.name == "git" then view.gutter = false end
    end)
end

function Git.init() end

return Git
