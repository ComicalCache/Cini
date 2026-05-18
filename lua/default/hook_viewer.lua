local HookViewer = {}

function HookViewer.setup()
    -- Faces.
    Core.Faces.register_face("hook_viewer.header", Core.Face({ fg = Core.Rgb(198, 120, 221), bold = true }))
    Core.Faces.register_face("hook_viewer.id", Core.Face({ fg = Core.Rgb(97, 175, 239) }))
    Core.Faces.register_face("hook_viewer.priority", Core.Face({ fg = Core.Rgb(235, 145, 70), italic = true }))
    Core.Faces.register_face("hook_viewer.desc", Core.Face({ fg = Core.Rgb(172, 178, 190) }))

    -- Mode.
    local current_line_override = Core.Faces.get_face("default") or {}
    Core.Modes.register_mode({
        name = "hook_viewer",
        faces = { current_line = Core.Face({ bg = current_line_override.bg }) },
        mode_line_layout = {
            { callback = function(_) return { { text = "Hook Viewer" } } end },
            "minor_mode_indicators",
            "pending_keys",
            "spacer",
            { callback = function(_) return { { text = "<Enter>: Expand/Collapse" } } end },
            "cursor_pos"
        },
        metadata = {
            read_only = true
        }
    })

    -- Hooks.
    Core.Hooks.add("cursor::after-move", {
            id = "hook_viewer.update",
            priority = 50,
            metadata = { description = "Updates the selected item after moving the cursor." }
        },
        function(view, _)
            local mode = Core.Modes.get_major_mode(view.doc)
            if mode and mode.name == "hook_viewer" then HookViewer.update_selection(view) end
        end)

    -- Commands.
    Core.Commands.register("global.hook_viewer", {
        metadata = {
            synopsis = "Open the hook viewer",
            description = "Opens a buffer listing all registered hooks and their properties."
        },
        callback = function() HookViewer.open() end
    })

    Core.Commands.register("hook_viewer.toggle", {
        metadata = {
            synopsis = "Toggles hook details",
            description = "Toggles more details of a hook like priority and description.",
        },
        callback = function()
            local view = Cini.workspace.viewport.view
            local hook_key = view.doc:get_text_property(view.cur:point(view), "hook_key")
            if not hook_key then return end

            local expanded = view.doc.properties["expanded_hooks"] or {}
            expanded[hook_key] = not expanded[hook_key]
            view.doc.properties["expanded_hooks"] = expanded

            HookViewer.refresh(view.doc)
        end
    })

    Core.Commands.register("hook_viewer.quit", {
        metadata = {
            synopsis = "Exits the hook viewer",
            description = "Exits the hook viewer, closing the buffer.",
        },
        callback = function() Cini:destroy_document(Cini.workspace.viewport.view.doc) end
    })

    -- Keybinds.
    Core.Keybinds.bind("global", "<M-h>", "global.hook_viewer")

    Core.Keybinds.bind("hook_viewer", "<Enter>", "hook_viewer.toggle")
    Core.Keybinds.bind("hook_viewer", "<C-q>", "hook_viewer.quit")
end

function HookViewer.init() end

function HookViewer.open()
    local doc = nil
    for _, d in ipairs(Cini.documents) do
        local mode = Core.Modes.get_major_mode(d)
        if mode and mode.name == "hook_viewer" then
            doc = d
            break
        end
    end

    if doc then
        if doc.properties["loaded"] then
            local vp = Cini.workspace:find_viewport(function(vp) return vp.view.doc == doc end)
            if vp then
                Cini.workspace:focus_viewport(vp)
                return
            end
        end

        Cini.workspace.viewport:change_document_view(Cini:create_document_view(doc))
        HookViewer.refresh(doc)
    else
        doc = Cini:create_document()
        doc.properties["name"] = "Hook Viewer"
        doc.properties["expanded_hooks"] = {}

        Cini.workspace.viewport:change_document_view(Cini:create_document_view(doc))
        Core.Modes.set_major_mode(doc, "hook_viewer")
        HookViewer.refresh(doc)
    end
end

--- @param doc Core.Document
function HookViewer.refresh(doc)
    local major_mode = Core.Modes.get_major_mode(doc)
    if not major_mode or major_mode.name ~= "hook_viewer" then return end

    local views = doc:views()
    local old_rows = {}
    for idx, view in ipairs(views) do
        old_rows[idx] = view.cur.row
        view:move_cursor(function(c, v) c:_jump_to_beginning_of_file(v) end, 0)
    end

    doc:clear()

    local expanded_state = doc.properties["expanded_hooks"] or {}

    local events = {}
    for event in pairs(Core.Hooks.registry) do table.insert(events, event) end
    table.sort(events)

    local first = true
    for _, event in ipairs(events) do
        local hooks = Core.Hooks.registry[event]

        -- Event Heading.
        local header_start = doc.size
        doc:insert(header_start, string.format("%s[%s]", first and "" or "\n\n", event))
        doc:add_text_property(header_start, doc.size, "face", "hook_viewer.header")
        first = false

        for _, hook in ipairs(hooks) do
            local hook_key = event .. ":" .. hook.id
            local prefix = expanded_state[hook_key] and "[-] " or "[+] "

            local line_start = doc.size
            local line_text = string.format("\n%s%s", prefix, hook.id)
            doc:insert(line_start, line_text)

            -- Plus five for skipping [+] or [-] with newline.
            local id_start = line_start + 5

            doc:add_text_property(line_start, doc.size, "hook_key", hook_key)
            doc:add_text_property(id_start, doc.size, "face", "hook_viewer.id")

            if expanded_state[hook_key] then
                local priority_start = doc.size
                local priority_text = string.format("\n    Priority: %d", hook.priority)
                doc:insert(priority_start, priority_text)
                doc:add_text_property(priority_start, doc.size, "face", "hook_viewer.priority")

                local desc =
                    (hook.metadata and hook.metadata.description) or
                    (hook.metadata and hook.metadata.synopsis) or
                    "No description."
                local desc_start = doc.size

                doc:insert(desc_start, string.format("\n    Description: %s", desc))
                doc:add_text_property(desc_start, doc.size, "face", "hook_viewer.desc")
            end
        end
    end

    doc.modified = false
    for idx, view in ipairs(views) do
        view:move_cursor(Core.Cursor.down, old_rows[idx] or 0)
        HookViewer.update_selection(view)
    end
end

--- @param view Core.DocumentView
function HookViewer.update_selection(view)
    local row = view.cur.row
    local start = view.doc:line_begin_byte(row)
    local stop = view.doc:line_end_byte(row)

    view:clear_view_properties("selection")
    if start ~= stop then
        local line_text = view.doc:slice(start, stop)

        local prefix = line_text:sub(0, 3)

        -- Only highlight on lines that can toggle details.
        if prefix == "[+]" or prefix == "[-]" then
            view:add_view_property(start, stop, "selection", "selection.selection")
        end
    end
end

return HookViewer
