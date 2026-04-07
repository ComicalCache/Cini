--- @class Core.DocumentViewer
local DocumentViewer = {}

function DocumentViewer.setup()
    -- Modes.
    local current_line_override = Core.Faces.get_face("default") or {}
    Core.Modes.register_mode({
        name = "document_viewer",
        faces = { current_line = Core.Face({ bg = current_line_override.bg }) },
        cursor_style = Core.CursorStyle.Hidden,
        mode_line = function(_)
            local ret = {}

            table.insert(ret, { text = " Document Viewer" })
            table.insert(ret, { spacer = true })
            table.insert(ret, { text = " <Enter>: Open | <C-c>: Close | <C-x>: Force Close " })

            return ret
        end
    })

    -- Hooks.
    Core.Hooks.add("document::created", 10, function() DocumentViewer.refresh() end)
    Core.Hooks.add("document::destroyed", 10, function() DocumentViewer.refresh() end)
    Core.Hooks.add("document::loaded", 10, function() DocumentViewer.refresh() end)
    Core.Hooks.add("document::unloaded", 10, function() DocumentViewer.refresh() end)

    Core.Hooks.add("command::before-execute", 10, function(_, cmd)
        --- @cast cmd Core.Command

        if Cini.workspace.is_mini_buffer then return true end

        local doc = Cini.workspace.viewport.view.doc
        local mode = Core.Modes.get_major_mode(doc)

        local legal = (cmd.metadata and cmd.metadata.modifies or cmd.metadata.changes_view)
        return not (legal and mode and mode.name == "document_viewer")
    end)

    Core.Hooks.add("cursor::after-move", 3, function(view, _)
        --- @cast view Core.DocumentView

        local mode = Core.Modes.get_major_mode(view.doc)
        if not mode or mode.name ~= "document_viewer" then return end

        DocumentViewer.update_selection(view)
    end)

    -- Commands.
    Core.Commands.register("global.open_document_viewer",
        { metadata = { changes_view = true }, run = function() DocumentViewer.open() end })

    Core.Commands.register("document_viewer.open_selected", {
        metadata = {},
        run = function()
            local target = DocumentViewer.get_selected_doc()
            if not target then return end

            local curr_doc = Cini.workspace.viewport.view.doc
            if curr_doc == target then return end

            if target.properties["loaded"] then
                local target_vp = Cini.workspace:find_viewport(function(vp) return vp.view.doc == target end)

                if target_vp then
                    Cini.workspace:close_split()
                    Cini.workspace:focus_viewport(target_vp)
                else
                    -- Fallback, should never happen.
                    Cini.workspace.viewport:change_document_view(Cini:create_document_view(target))
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
            local target = DocumentViewer.get_selected_doc()
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
                    DocumentViewer.refresh()
                end
            end)
        end
    })

    Core.Commands.register("document_viewer.force_close_selected", {
        metadata = {},
        run = function()
            local target = DocumentViewer.get_selected_doc()
            if not target then return end

            local name = target.path or "Scratchpad"
            Core.Prompt.run("Force close " .. name .. "? (y/n) ", nil, function(sel)
                if sel:lower() == "y" then
                    Cini:destroy_document(target)
                    DocumentViewer.refresh()
                end
            end)
        end
    })

    Core.Commands.register("document_viewer.quit", {
        metadata = {}, run = function() Cini:destroy_document(Cini.workspace.viewport.view.doc) end })

    -- Keybinds.
    Core.Keybinds.bind("global", "<C-b>", "global.open_document_viewer")
    Core.Keybinds.bind("document_viewer", "<Enter>", "document_viewer.open_selected")
    Core.Keybinds.bind("document_viewer", "<C-c>", "document_viewer.close_selected")
    Core.Keybinds.bind("document_viewer", "<C-x>", "document_viewer.force_close_selected")
    Core.Keybinds.bind("document_viewer", "<C-q>", "document_viewer.quit")
end

function DocumentViewer.init() end

function DocumentViewer.open()
    local doc = Cini:create_document()
    doc.properties["name"] = "Document Viewer"
    Core.Modes.set_major_mode(doc, "document_viewer")

    Cini.workspace.viewport:change_document_view(Cini:create_document_view(doc))
    DocumentViewer.refresh()
end

--- @return Core.Document?
function DocumentViewer.get_selected_doc()
    local view = Cini.workspace.viewport.view
    return view.doc:get_text_property(view.cur:point(view), "target_doc")
end

function DocumentViewer.refresh()
    local view = Cini.workspace.viewport.view
    local major_mode = Core.Modes.get_major_mode(view.doc)
    if not major_mode or major_mode.name ~= "document_viewer" then return end

    local doc = view.doc

    local old_row = view.cur.row
    view:move_cursor(function(c, v, _) c:_jump_to_beginning_of_file(v) end, 0)

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

        local line_text = string.format("%s%s: [%s] %s", first and "" or "\n", loaded, modified, filename)
        first = false

        local start = doc.size
        doc:insert(start, line_text)

        doc:add_text_property(start, doc.size, "target_doc", d)
    end

    for _, d in ipairs(background) do write(d, true) end
    if #background > 0 and #foreground > 0 then doc:insert(doc.size, "\n") end
    for _, d in ipairs(foreground) do write(d, false) end

    view:move_cursor(Core.Cursor.down, old_row)
    DocumentViewer.update_selection(view)
end

--- @param view Core.DocumentView
function DocumentViewer.update_selection(view)
    local row = view.cur.row

    local start_byte = view.doc:line_begin_byte(row)
    local end_byte = view.doc:line_end_byte(row)

    -- Clear previous highlight.
    view:clear_view_properties("selection")
    if start_byte ~= end_byte then
        view:add_view_property(start_byte, end_byte, "selection", "selection")
    end
end

return DocumentViewer
