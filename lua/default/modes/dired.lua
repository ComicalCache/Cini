local Dired = {}

function Dired.setup()
    -- Faces.
    Core.Faces.register_face("dired.dir", Core.Face({ fg = Core.Rgb(97, 175, 239) }))
    Core.Faces.register_face("dired.symlink", Core.Face({ fg = Core.Rgb(198, 120, 221) }))
    Core.Faces.register_face("dired.info", Core.Face({ fg = Core.Rgb(126, 128, 130) }))

    -- Modes.
    local current_line_override = Core.Faces.get_face("default") or {}
    Core.Modes.register_mode({
        name = "dired",
        faces = { current_line = Core.Face({ bg = current_line_override.bg }) },
        cursor_style = Core.CursorStyle.Hidden,
        mode_line = function(viewport)
            return {
                { text = (viewport.view.doc.properties["dired_directory"] or "") },
                { spacer = true },
                { text = " <Enter>: Open | <C-r>: Refresh " }
            }
        end
    })

    -- Hooks.
    Core.Hooks.add("command::before-execute", 50, function(_, cmd)
        --- @cast cmd Core.Command

        if Cini.workspace.is_mini_buffer then return true end

        local doc = Cini.workspace.viewport.view.doc
        local mode = Core.Modes.get_major_mode(doc)

        local legal = (cmd.metadata and cmd.metadata.modifies)
        return not (legal and mode and mode.name == "dired")
    end)
    Core.Hooks.add("cursor::after-move", 50, function(view, _)
        --- @cast view Core.DocumentView

        local mode = Core.Modes.get_major_mode(view.doc)
        if not mode or mode.name ~= "dired" then return end

        Dired.update_selection(view)
    end)

    -- Commands.
    Core.Commands.register("global.dired", {
        metadata = { changes_view = true },
        run = function() Dired.open() end
    })

    Core.Commands.register("dired.refresh", {
        metadata = {},
        run = function()
            local doc = Cini.workspace.viewport.view.doc
            Dired.refresh(doc, doc.properties["dired_directory"])
        end
    })
    Core.Commands.register("dired.open_selected", {
        metadata = {},
        run = function()
            local view = Cini.workspace.viewport.view
            local point = view.cur:point(view)

            local path = view.doc:get_text_property(point, "path")
            if not path then return end

            if view.doc:get_text_property(point, "is_dir") then
                Dired.refresh(view.doc, path)
            else
                Cini.workspace.viewport:change_document_view(Cini:create_document_view(Cini:create_document(path)))
            end
        end
    })

    -- Keybinds.
    Core.Keybinds.bind("global", "<C-d>", "global.dired")
    Core.Keybinds.bind("global", "<C-r>", "dired.refresh")
    Core.Keybinds.bind("dired", "<Enter>", "dired.open_selected")
end

function Dired.init() end

function Dired.open()
    local doc = nil
    for _, d in ipairs(Cini.documents) do
        local mode = Core.Modes.get_major_mode(d)
        if mode and mode.name == "dired" then
            doc = d
            break
        end
    end

    local dir = doc and doc.properties["dired_directory"] or os.getenv("PWD") or "/"

    if doc then -- Dired already exists.
        if doc.properties["loaded"] then
            local vp = Cini.workspace:find_viewport(function(vp) return vp.view.doc == doc end)
            if vp then
                Cini.workspace:focus_viewport(vp)
                Dired.refresh(doc, dir)
                return
            end
        end

        local view = Cini:create_document_view(doc)
        view.properties["ws"] = nil
        view.properties["nl"] = nil
        view.properties["tab"] = nil

        Cini.workspace.viewport:change_document_view(view)
        Dired.refresh(doc, dir)
    else -- Create new Dired.
        doc = Cini:create_document()
        Core.Modes.set_major_mode(doc, "dired")

        local view = Cini:create_document_view(doc)
        view.properties["ws"] = nil
        view.properties["nl"] = nil
        view.properties["tab"] = nil

        Cini.workspace.viewport:change_document_view(view)
        Dired.refresh(doc, dir)
    end
end

--- @param doc Core.Document
--- @param dir string
function Dired.refresh(doc, dir)
    if string.sub(dir, -1) ~= "/" then dir = dir .. "/" end
    doc.properties["dired_directory"] = dir
    doc.properties["name"] = "Dired: " .. dir

    doc:clear()

    local cmd = string.format('LC_ALL=C ls -alhp "%s"', dir)
    local p = io.popen(cmd)
    if not p then return end

    local first = true
    for line in p:lines() do
        if not line:match("^total ") then
            --- @type string, string
            local prefix, filename = line:match("^(.-%s+%d%d?%s+[%d:]+)%s+(.+)$")

            if prefix and filename then
                local is_dir = line:sub(1, 1) == "d"

                local path = dir .. filename

                -- Resolve directory traversial.
                if filename == "../" then
                    path = dir:gsub("[^/]+/?$", "")
                    if path == "" then path = "/" end
                elseif filename == "./" then
                    path = dir
                end

                local start = doc.size
                doc:insert(start, string.format("%s%s", first and "" or "\n", line))

                doc:add_text_property(start, doc.size, "path", path)
                doc:add_text_property(start, doc.size, "is_dir", is_dir)

                start = first and start or (start + 1)
                local info_stop = start + #prefix

                doc:add_text_property(start, info_stop, "face", "dired.info")

                if is_dir then
                    doc:add_text_property(info_stop, doc.size, "face", "dired.dir")
                elseif line:sub(1, 1) == "l" then
                    doc:add_text_property(info_stop, doc.size, "face", "dired.symlink")
                end

                first = false
            end
        end
    end
    p:close()

    for _, view in ipairs(doc:views()) do
        view:move_cursor(function(c, v, _) c:_jump_to_beginning_of_file(v) end, 0)
        Dired.update_selection(view)
    end

    doc.modified = false
end

--- @param view Core.DocumentView
function Dired.update_selection(view)
    local row = view.cur.row

    local start = view.doc:line_begin_byte(row)
    local stop = view.doc:line_end_byte(row)

    view:clear_view_properties("selection")
    if start ~= stop then
        view:add_view_property(start, stop, "selection", "selection.selection")
    end
end

return Dired
