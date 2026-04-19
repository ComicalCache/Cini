local ProcessViewer = {}

function ProcessViewer.setup()
    -- Modes.
    local current_line_override = Core.Faces.get_face("default") or {}
    Core.Modes.register_mode({
        name = "process_viewer",
        faces = { current_line = Core.Face({ bg = current_line_override.bg }) },
        cursor_style = Core.CursorStyle.Hidden,
        mode_line_layout = {
            { run = function(_) return { { text = "Process Viewer" } } end },
            "pending_keys",
            "spacer",
            {
                run = function(_)
                    return { { text = "<C-x>: Kill | <C-r>: Refresh" } }
                end
            },
        }
    })

    -- Hooks.
    local function refresh()
        for _, doc in ipairs(Cini.documents) do
            local mode = Core.Modes.get_major_mode(doc)
            if mode and mode.name == "process_viewer" then
                ProcessViewer.refresh(doc)
            end
        end
    end

    Core.Hooks.add("command::before-execute", 50, function(_, cmd)
        --- @cast cmd Core.Command

        if Cini.workspace.is_mini_buffer then return true end

        local mode = Core.Modes.get_major_mode(Cini.workspace.viewport.view.doc)

        local legal = (cmd.metadata and cmd.metadata.modifies)
        return not (legal and mode and mode.name == "process_viewer")
    end)

    Core.Hooks.add("cursor::after-move", 50, function(view, _)
        --- @cast view Core.DocumentView

        local mode = Core.Modes.get_major_mode(view.doc)
        if not mode or mode.name ~= "process_viewer" then return end

        ProcessViewer.update_selection(view)
    end)

    Core.Hooks.add("document::set-major-mode", 50, function(doc, mode)
        --- @cast doc Core.Document
        --- @cast mode string

        if mode ~= "process_viewer" then return end

        for _, view in ipairs(doc:views()) do
            view.properties["ws"] = nil
            view.properties["nl"] = nil
            view.properties["tab"] = nil
        end
    end)

    Core.Hooks.add("document_view::created", 50, function(view)
        --- @cast view Core.DocumentView

        local mode = Core.Modes.get_major_mode(view.doc)
        if mode and mode.name == "process_viewer" then
            view.properties["ws"] = nil
            view.properties["nl"] = nil
            view.properties["tab"] = nil
        end
    end)

    Core.Hooks.add("process::spawned", 50, function(_) refresh() end)
    Core.Hooks.add("process::exited", 50, function(_, _) refresh() end)

    -- Commands.
    Core.Commands.register("global.process_viewer", {
        metadata = {},
        run = function() ProcessViewer.open() end
    })

    Core.Commands.register("process_viewer.refresh", {
        metadata = {},
        run = function() refresh() end
    })
    Core.Commands.register("process_viewer.kill_selected", {
        metadata = {},
        run = function()
            local view = Cini.workspace.viewport.view
            local process = ProcessViewer.get_selected_process(view)
            if not process then return end

            Core.Prompt.run("Kill process '" .. process.command .. "'? (y/n) ", nil, function(sel)
                if sel:lower() == "y" then process:kill() end
            end)
        end
    })
    Core.Commands.register("process_viewer.quit", {
        metadata = {},
        run = function() Cini:destroy_document(Cini.workspace.viewport.view.doc) end
    })

    -- Keybinds.
    Core.Keybinds.bind("global", "<C-l>", "global.process_viewer")

    Core.Keybinds.bind("process_viewer", "<C-r>", "process_viewer.refresh")
    Core.Keybinds.bind("process_viewer", "<C-x>", "process_viewer.kill_selected")
    Core.Keybinds.bind("process_viewer", "<C-q>", "process_viewer.quit")
end

function ProcessViewer.init() end

function ProcessViewer.open()
    local doc = nil
    for _, d in ipairs(Cini.documents) do
        local mode = Core.Modes.get_major_mode(d)
        if mode and mode.name == "process_viewer" then
            doc = d
            break
        end
    end

    if doc then -- ProcessViewer already exists.
        if doc.properties["loaded"] then
            local vp = Cini.workspace:find_viewport(function(vp) return vp.view.doc == doc end)
            if vp then
                Cini.workspace:focus_viewport(vp)
                ProcessViewer.refresh(doc)
                return
            end
        end

        local view = Cini:create_document_view(doc)
        view.properties["ws"] = nil
        view.properties["nl"] = nil
        view.properties["tab"] = nil

        Cini.workspace.viewport:change_document_view(view)
        ProcessViewer.refresh(doc)
    else -- Create new ProcessViewer.
        doc = Cini:create_document()
        doc.properties["name"] = "Process Viewer"

        Cini.workspace.viewport:change_document_view(Cini:create_document_view(doc))
        Core.Modes.set_major_mode(doc, "process_viewer")

        ProcessViewer.refresh(doc)
    end
end

--- @param view Core.DocumentView
--- @return Core.AsyncProcess?
function ProcessViewer.get_selected_process(view)
    return view.doc:get_text_property(view.cur:point(view), "process")
end

--- @param doc Core.Document?
function ProcessViewer.refresh(doc)
    if not doc then return end

    local major_mode = Core.Modes.get_major_mode(doc)
    if not major_mode or major_mode.name ~= "process_viewer" then return end

    local views = doc:views()
    local old_rows = {}
    for idx, view in ipairs(views) do
        old_rows[idx] = view.cur.row
        view:move_cursor(function(c, v, _) c:_jump_to_beginning_of_file(v) end, 0)
    end

    doc:clear()
    doc.modified = false

    if #Cini.processes == 0 then
        doc:insert(0, "No active processes.")
        doc:add_text_property(0, doc.size, "face", "document_viewer.background")
    else
        local first = true
        for _, process in ipairs(Cini.processes) do
            local text = string.format("%s[%s] -> %s", first and "" or "\n", process.command,
                process.doc.properties["name"] or process.doc.path or "Scratchpad")

            local start = doc.size
            doc:insert(start, text)
            doc:add_text_property(start, doc.size, "process", process)

            start = first and start or (start + 1)
            local stop = start + 1
            doc:add_text_property(start, stop, "face", "document_viewer.foreground")

            first = false
        end
    end

    doc.modified = false

    for idx, view in ipairs(views) do
        view:move_cursor(Core.Cursor.down, old_rows[idx] or 0)
        ProcessViewer.update_selection(view)
    end
end

--- @param view Core.DocumentView
function ProcessViewer.update_selection(view)
    local row = view.cur.row

    local start = view.doc:line_begin_byte(row)
    local stop = view.doc:line_end_byte(row)

    view:clear_view_properties("selection")
    if start ~= stop then
        view:add_view_property(start, stop, "selection", "selection.selection")
    end
end

return ProcessViewer
