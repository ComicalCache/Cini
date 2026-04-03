--- @class Core.DoczmentViewer
local DocumentViewer = {}

function DocumentViewer.init()
    Core.Modes.register_mode(Core.Mode.new({
        name = "document_viewer",
        cursor_style = Core.CursorStyle.SteadyBlock
    }))

    Core.Hooks.add("document::created", 10, function() DocumentViewer.refresh() end)
    Core.Hooks.add("document::destroyed", 10, function() DocumentViewer.refresh() end)
    Core.Hooks.add("document::loaded", 10, function() DocumentViewer.refresh() end)
    Core.Hooks.add("document::unloaded", 10, function() DocumentViewer.refresh() end)

    Core.Hooks.add("command::before-execute", 10, function(_, cmd)
        if Cini.workspace.is_mini_buffer then return true end

        local doc = Cini.workspace.viewport.doc
        local mode = Core.Modes.get_major_mode(doc)
        return not (cmd.metadata.modifies and mode and mode.name == "document_viewer")
    end)

    -- Open selected Document.
    Core.Commands.register("document_viewer.open_selected", {
        metadata = { modifies = false },
        run = function()
            local target_doc = DocumentViewer.get_selected_doc()
            if not target_doc then return end

            local curr_doc = Cini.workspace.viewport.doc
            if curr_doc == target_doc then return end

            if target_doc.properties["loaded"] then
                local target_vp = Cini.workspace:find_viewport(function(vp) return vp.doc == target_doc end)

                if target_vp then
                    Cini.workspace:close_split()
                    Cini.workspace:focus_viewport(target_vp)
                else
                    -- Fallback, should never happen.
                    Cini.workspace.viewport:change_document(target_doc)
                end
            else
                Cini.workspace.viewport:change_document(target_doc)
            end

            Cini:destroy_document(curr_doc)
        end
    })

    -- Close.
    Core.Commands.register("document_viewer.close_selected", {
        metadata = { modifies = false },
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

    -- Force close.
    Core.Commands.register("document_viewer.force_close_selected", {
        metadata = { modifies = false },
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

    Core.Keybinds.bind("document_viewer", "<Enter>", "document_viewer.open_selected")
    Core.Keybinds.bind("document_viewer", "<C-c>", "document_viewer.close_selected")
    Core.Keybinds.bind("document_viewer", "<C-x>", "document_viewer.force_close_selected")

    Core.DocumentViewer = DocumentViewer
end

function DocumentViewer.get_selected_doc()
    local viewport = Cini.workspace.viewport
    return viewport.doc:get_text_property(viewport.cursor:point(viewport.doc), "target_doc")
end

function DocumentViewer.refresh()
    local viewport = Cini.workspace.viewport
    if not viewport then return end

    local major_mode = Core.Modes.get_major_mode(viewport.doc)
    if not major_mode or major_mode.name ~= "document_viewer" then return end

    local doc = viewport.doc

    local old_row = viewport.cursor.row
    viewport:move_cursor(function(c, d, _) c:_jump_to_beginning_of_file(d) end, 0)

    doc:clear()
    doc.modified = false

    for _, d in ipairs(Cini.documents) do
        local filename = d.properties["name"] or d.path or "Scratchpad"
        local mod_stat = d.modified and "*" or " "
        local load_stat = d.properties["loaded"] and "Foreground" or "Background"

        local line_text = string.format("%s: [%s] %s\n", load_stat, mod_stat, filename)
        local start = doc.size
        doc:insert(start, line_text)

        doc:add_text_property(start, doc.size, "target_doc", d)

        -- Prevent this document to appear as edited in the list.
        doc.modified = false
    end

    viewport:move_cursor(Core.Cursor.down, old_row)
end

function DocumentViewer.open()
    local doc = Cini:create_document()
    doc.properties["name"] = "Document Viewer"
    Core.Modes.set_major_mode(doc, "document_viewer")

    Cini.workspace.viewport:change_document(doc)
    DocumentViewer.refresh()
end

return DocumentViewer
