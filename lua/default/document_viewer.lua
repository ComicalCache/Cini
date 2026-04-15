--- @class Core.DocumentViewer
local DocumentViewer = {}

function DocumentViewer.setup()
    -- Faces.
    Core.Faces.register_face("document_viewer.foreground", Core.Face({ fg = Core.Rgb(97, 175, 239) }))
    Core.Faces.register_face("document_viewer.background", Core.Face({ fg = Core.Rgb(126, 128, 130) }))
    Core.Faces.register_face("document_viewer.modified", Core.Face({ fg = Core.Rgb(224, 108, 117) }))

    -- Modes.
    local current_line_override = Core.Faces.get_face("default") or {}
    Core.Modes.register_mode({
        name = "document_viewer",
        faces = { current_line = Core.Face({ bg = current_line_override.bg }) },
        cursor_style = Core.CursorStyle.Hidden,
        mode_line_layout = {
            { run = function(_) return { { text = "Document Viewer" } } end },
            "pending_keys",
            "spacer",
            {
                run = function(_)
                    return { { text = "<Enter>: Open | <C-c>: Close | <C-x>: Force Close | <C-r>: Refresh" } }
                end
            },
        }
    })

    -- Hooks.
    local function refresh()
        for _, d in ipairs(Cini.documents) do
            local mode = Core.Modes.get_major_mode(d)
            if mode and mode.name == "document_viewer" then
                DocumentViewer.refresh(d)
            end
        end
    end

    Core.Hooks.add("command::before-execute", 50, function(_, cmd)
        --- @cast cmd Core.Command

        if Cini.workspace.is_mini_buffer then return true end

        local doc = Cini.workspace.viewport.view.doc
        local mode = Core.Modes.get_major_mode(doc)

        local legal = (cmd.metadata and cmd.metadata.modifies or cmd.metadata.changes_view)
        return not (legal and mode and mode.name == "document_viewer")
    end)

    Core.Hooks.add("cursor::after-move", 50, function(view, _)
        --- @cast view Core.DocumentView

        local mode = Core.Modes.get_major_mode(view.doc)
        if not mode or mode.name ~= "document_viewer" then return end

        DocumentViewer.update_selection(view)
    end)

    Core.Hooks.add("document::created", 50, function(_) refresh() end)
    Core.Hooks.add("document::destroyed", 50, function(_) refresh() end)
    Core.Hooks.add("document::loaded", 50, function(_) refresh() end)
    Core.Hooks.add("document::unloaded", 50, function(_) refresh() end)

    Core.Hooks.add("document::set-major-mode", 50, function(doc, mode)
        --- @cast doc Core.Document
        --- @cast mode string

        if mode ~= "document_viewer" then return end

        for _, view in ipairs(doc:views()) do
            view.properties["ws"] = nil
            view.properties["nl"] = nil
            view.properties["tab"] = nil
        end
    end)

    Core.Hooks.add("document_view::created", 50, function(view)
        --- @cast view Core.DocumentView

        local mode = Core.Modes.get_major_mode(view.doc)

        if mode and mode.name == "document_viewer" then
            view.properties["ws"] = nil
            view.properties["nl"] = nil
            view.properties["tab"] = nil
        end
    end)

    -- Commands.
    Core.Commands.register("global.document_viewer", {
        metadata = { changes_view = true },
        run = function() DocumentViewer.open() end
    })

    Core.Commands.register("document_viewer.refresh", {
        metadata = {},
        run = function() DocumentViewer.refresh(Cini.workspace.viewport.view.doc) end
    })
    Core.Commands.register("document_viewer.open_selected", {
        metadata = {},
        run = function()
            local view = Cini.workspace.viewport.view
            local target = DocumentViewer.get_selected_doc(view)
            if not target then return end

            local curr_doc = Cini.workspace.viewport.view.doc
            if curr_doc == target then return end

            if target.properties["loaded"] then
                local vp = Cini.workspace:find_viewport(function(vp) return vp.view.doc == target end)

                if vp then
                    Cini.workspace:close_split()
                    Cini.workspace:focus_viewport(vp)
                end
            else
                Cini.workspace.viewport:change_document_view(Cini:create_document_view(target))
            end

            Cini:destroy_document(curr_doc)
        end
    })

    Core.Commands.register("document_viewer.close_selected", {
        metadata = {},
        run = function()
            local view = Cini.workspace.viewport.view
            local target = DocumentViewer.get_selected_doc(view)
            if not target then return end

            if target.modified then
                Cini:set_status_message("Document has unsaved changes. Use <C-x> to force close.", "error_message", 0,
                    false)
                return
            end

            local name = target.path or "Scratchpad"
            Core.Prompt.run("Close " .. name .. "? (y/n) ", nil, function(sel)
                if sel:lower() == "y" then
                    Cini:destroy_document(target)
                    DocumentViewer.refresh(view.doc)
                end
            end)
        end
    })
    Core.Commands.register("document_viewer.force_close_selected", {
        metadata = {},
        run = function()
            local view = Cini.workspace.viewport.view
            local target = DocumentViewer.get_selected_doc(view)
            if not target then return end

            local name = target.path or "Scratchpad"
            Core.Prompt.run("Force close " .. name .. "? (y/n) ", nil, function(sel)
                if sel:lower() == "y" then
                    Cini:destroy_document(target)
                    DocumentViewer.refresh(view.doc)
                end
            end)
        end
    })

    Core.Commands.register("document_viewer.quit", {
        metadata = {}, run = function() Cini:destroy_document(Cini.workspace.viewport.view.doc) end })

    -- Keybinds.
    Core.Keybinds.bind("global", "<C-b>", "global.document_viewer")
    Core.Keybinds.bind("document_viewer", "<C-r>", "document_viewer.refresh")
    Core.Keybinds.bind("document_viewer", "<Enter>", "document_viewer.open_selected")
    Core.Keybinds.bind("document_viewer", "<C-c>", "document_viewer.close_selected")
    Core.Keybinds.bind("document_viewer", "<C-x>", "document_viewer.force_close_selected")
    Core.Keybinds.bind("document_viewer", "<C-q>", "document_viewer.quit")
end

function DocumentViewer.init() end

function DocumentViewer.open()
    local doc = nil
    for _, d in ipairs(Cini.documents) do
        local mode = Core.Modes.get_major_mode(d)
        if mode and mode.name == "document_viewer" then
            doc = d
            break
        end
    end

    if doc then -- DocumentViewer already exists.
        if doc.properties["loaded"] then
            local vp = Cini.workspace:find_viewport(function(vp) return vp.view.doc == doc end)
            if vp then
                Cini.workspace:focus_viewport(vp)
                DocumentViewer.refresh(doc)
                return
            end
        end

        local view = Cini:create_document_view(doc)
        view.properties["ws"] = nil
        view.properties["nl"] = nil
        view.properties["tab"] = nil

        Cini.workspace.viewport:change_document_view(view)
        DocumentViewer.refresh(doc)
    else -- Create new DocumentViewer.
        doc = Cini:create_document()
        doc.properties["name"] = "Document Viewer"

        Cini.workspace.viewport:change_document_view(Cini:create_document_view(doc))
        Core.Modes.set_major_mode(doc, "document_viewer")

        DocumentViewer.refresh(doc)
    end
end

--- @param view Core.DocumentView
--- @return Core.Document?
function DocumentViewer.get_selected_doc(view)
    return view.doc:get_text_property(view.cur:point(view), "doc")
end

--- @param doc Core.Document?
function DocumentViewer.refresh(doc)
    if not doc then return end

    local major_mode = Core.Modes.get_major_mode(doc)
    if not major_mode or major_mode.name ~= "document_viewer" then return end

    local views = doc:views()
    local old_rows = {}
    for idx, view in ipairs(views) do
        old_rows[idx] = view.cur.row
        view:move_cursor(function(c, v, _) c:_jump_to_beginning_of_file(v) end, 0)
    end

    doc:clear()
    doc.modified = false

    local background = {}
    local foreground = {}

    for _, d in ipairs(Cini.documents) do
        -- Avoid a circular reference to itself causing dangling Documents.
        if d ~= doc then
            if d.properties["loaded"] then
                table.insert(foreground, d)
            else
                table.insert(background, d)
            end
        end
    end

    local first = true

    -- Helper to format and insert a document line
    local function write(d, bg)
        local filename = d.properties["name"] or d.path or "Scratchpad"
        local modified = d.modified and "*" or " "
        local loaded = bg and "Background" or "Foreground"

        local text = string.format("%s%s: [%s] %s", first and "" or "\n", loaded, modified, filename)

        local start = doc.size
        doc:insert(start, text)

        doc:add_text_property(start, doc.size, "doc", d)

        start = first and start or (start + 1)
        local stop = start + #loaded
        doc:add_text_property(start, stop, "face",
            bg and "document_viewer.background" or "document_viewer.foreground")

        start = stop + 3
        stop = start + 1
        if d.modified then doc:add_text_property(start, stop, "face", "document_viewer.modified") end

        first = false
    end

    for _, d in ipairs(background) do write(d, true) end
    if #background > 0 and #foreground > 0 then doc:insert(doc.size, "\n") end
    for _, d in ipairs(foreground) do write(d, false) end

    for idx, view in ipairs(views) do
        view:move_cursor(Core.Cursor.down, old_rows[idx] or 0)
        DocumentViewer.update_selection(view)
    end
end

--- @param view Core.DocumentView
function DocumentViewer.update_selection(view)
    local row = view.cur.row

    local start = view.doc:line_begin_byte(row)
    local stop = view.doc:line_end_byte(row)

    -- Clear previous highlight.
    view:clear_view_properties("selection")
    if start ~= stop then
        view:add_view_property(start, stop, "selection", "selection.selection")
    end
end

return DocumentViewer
