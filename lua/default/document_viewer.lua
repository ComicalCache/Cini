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
            { callback = function(_) return { { text = "Document Viewer" } } end },
            "minor_mode_indicators",
            "pending_keys",
            "spacer",
            {
                callback = function(_)
                    return { { text = "<Enter>: Open | <C-c>: Close | <C-x>: Force Close | <C-r>: Refresh" } }
                end
            },
        },
        metadata = {
            read_only = true
        }
    })

    -- Hooks.
    local function refresh()
        for _, doc in ipairs(Cini.documents) do
            local mode = Core.Modes.get_major_mode(doc)
            if mode and mode.name == "document_viewer" then
                DocumentViewer.refresh(doc)
            end
        end

        Cini:request_render()
    end

    Core.Hooks.add("cursor::after-move", {
            id = "document_viewer.update",
            priority = 50,
            metadata = { description = "Updates the selected item after moving the cursor." }
        },
        function(view, _)
            local mode = Core.Modes.get_major_mode(view.doc)
            if mode and mode.name == "document_viewer" then DocumentViewer.update_selection(view) end
        end)

    Core.Hooks.add("document::created", {
            id = "document_viewer.document_created",
            priority = 50,
            metadata = { description = "Refreshes the list of Documents." }
        },
        function(_) refresh() end)
    Core.Hooks.add("document::destroyed", {
            id = "document_viewer.document_destroyed",
            priority = 50,
            metadata = { description = "Refreshes the list of Documents." }
        },
        function(_) refresh() end)
    Core.Hooks.add("document::loaded", {
            id = "document_viewer.document_loaded",
            priority = 50,
            metadata = { description = "Refreshes the list of Documents." }
        },
        function(_) refresh() end)
    Core.Hooks.add("document::unloaded", {
            id = "document_viewer.document_unloaded",
            priority = 50,
            metadata = { description = "Refreshes the list of Documents." }
        },
        function(_) refresh() end)

    -- Commands.
    Core.Commands.register("global.document_viewer", {
        metadata = {
            synopsis = "Open the document viewer",
            description = "Opens a buffer listing all opened documents in the foreground and background.",
        },
        callback = function() DocumentViewer.open() end
    })

    Core.Commands.register("document_viewer.refresh", {
        metadata = {
            synopsis = "Refreshes the document viewer",
            description = "Refreshes the list of shown documents to reflect newly created or closed documents.",
        },
        callback = function() refresh() end
    })
    Core.Commands.register("document_viewer.open_selected", {
        metadata = {
            synopsis = "Opens the selected document",
            description = "Opens the selected document in a view.",
        },
        callback = function()
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
        metadata = {
            synopsis = "Close the selected document",
            description = "Close the selected document if there are no pending changes.",
        },
        callback = function()
            local view = Cini.workspace.viewport.view
            local target = DocumentViewer.get_selected_doc(view)
            if not target then return end

            if target.modified then
                Cini:set_status_message("Document has unsaved changes. Use <C-x> to force close.", "error_message",
                    3000, false)
                return
            end

            local name = target.properties["name"] or target.path or "Scratchpad"
            Core.Prompt.run("Close " .. name .. "? (y/n) ", nil, function(sel)
                if sel:lower() == "y" then
                    Cini:destroy_document(target)
                    refresh()
                end
            end)
        end
    })
    Core.Commands.register("document_viewer.force_close_selected", {
        metadata = {
            synopsis = "Force-close the selected document",
            description = "Force-close the selected document discarding pending changes.",
        },
        callback = function()
            local view = Cini.workspace.viewport.view
            local target = DocumentViewer.get_selected_doc(view)
            if not target then return end

            local name = target.properties["name"] or target.path or "Scratchpad"
            Core.Prompt.run("Force close " .. name .. "? (y/n) ", nil, function(sel)
                if sel:lower() == "y" then
                    Cini:destroy_document(target)
                    refresh()
                end
            end)
        end
    })

    Core.Commands.register("document_viewer.quit", {
        metadata = {
            synopsis = "Exits the document viewer",
            description = "Exits the document viewer, closing the buffer.",
        },
        callback = function() Cini:destroy_document(Cini.workspace.viewport.view.doc) end
    })

    -- Keybinds.
    Core.Keybinds.bind("global", "<M-b>", "global.document_viewer")
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

        Cini.workspace.viewport:change_document_view(Cini:create_document_view(doc))
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

--- @param doc Core.Document
function DocumentViewer.refresh(doc)
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

    view:clear_view_properties("selection")
    if start ~= stop then view:add_view_property(start, stop, "selection", "selection.selection") end
end

return DocumentViewer
