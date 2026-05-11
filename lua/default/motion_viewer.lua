local MotionViewer = {}

function MotionViewer.setup()
    -- Faces.
    Core.Faces.register_face("motion_viewer.name", Core.Face({ fg = Core.Rgb(97, 175, 239), bold = true }))
    Core.Faces.register_face("motion_viewer.sequence", Core.Face({ fg = Core.Rgb(235, 145, 70), bold = true }))
    Core.Faces.register_face("motion_viewer.synopsis", Core.Face({ fg = Core.Rgb(126, 128, 130), italic = true }))
    Core.Faces.register_face("motion_viewer.desc", Core.Face({ fg = Core.Rgb(172, 178, 190) }))
    Core.Faces.register_face("motion_viewer.generated", Core.Face({ fg = Core.Rgb(198, 120, 221) }))

    -- Mode.
    local current_line_override = Core.Faces.get_face("default") or {}
    Core.Modes.register_mode({
        name = "motion_viewer",
        faces = { current_line = Core.Face({ bg = current_line_override.bg }) },
        mode_line_layout = {
            { run = function(_) return { { text = "Motion Viewer" } } end },
            "minor_mode_indicators",
            "pending_keys",
            "spacer",
            { run = function(_) return { { text = "<Enter>: Expand/Collapse" } } end },
            "cursor_pos"
        }
    })

    -- Hooks.
    Core.Hooks.add("command::before-execute", 50, function(_, cmd)
        --- @cast cmd Core.Command

        if Cini.workspace.is_mini_buffer then return true end

        local mode = Core.Modes.get_major_mode(Cini.workspace.viewport.view.doc)
        local legal = (cmd.metadata and cmd.metadata.modifies)
        return not (legal and mode and mode.name == "motion_viewer")
    end)

    Core.Hooks.add("cursor::after-move", 50, function(view, _)
        local mode = Core.Modes.get_major_mode(view.doc)
        if mode and mode.name == "motion_viewer" then MotionViewer.update_selection(view) end
    end)

    Core.Hooks.add("document::set-major-mode", 50, function(doc, mode)
        --- @cast doc Core.Document
        --- @cast mode string

        if mode ~= "motion_viewer" then return end

        for _, view in ipairs(doc:views()) do
            view.properties["ws"] = nil
            view.properties["nl"] = nil
            view.properties["tab"] = nil
        end
    end)

    Core.Hooks.add("document_view::created", 50, function(view)
        --- @cast view Core.DocumentView

        local mode = Core.Modes.get_major_mode(view.doc)

        if mode and mode.name == "motion_viewer" then
            view.properties["ws"] = nil
            view.properties["nl"] = nil
            view.properties["tab"] = nil
        end
    end)

    -- Commands.
    Core.Commands.register("global.motion_viewer", {
        metadata = {
            synopsis = "Open the motion viewer",
            description = "Opens a buffer listing all registered motions and their properties."
        },
        run = function() MotionViewer.open() end
    })

    Core.Commands.register("motion_viewer.toggle", {
        metadata = {
            synopsis = "Toggles motion details",
            description = "Toggles more details of a motion like a full description.",
        },
        run = function()
            local view = Cini.workspace.viewport.view
            local motion_name = view.doc:get_text_property(view.cur:point(view), "motion_name")
            if not motion_name then return end

            local expanded = view.doc.properties["expanded_motions"] or {}
            expanded[motion_name] = not expanded[motion_name]
            view.doc.properties["expanded_motions"] = expanded

            MotionViewer.refresh(view.doc)
        end
    })

    Core.Commands.register("motion_viewer.quit", {
        metadata = {
            synopsis = "Exits the motion viewer",
            description = "Exits the motion viewer, closing the buffer.",
        },
        run = function() Cini:destroy_document(Cini.workspace.viewport.view.doc) end
    })

    -- Keybinds.
    Core.Keybinds.bind("global", "<M-m>", "global.motion_viewer")

    Core.Keybinds.bind("motion_viewer", "<Enter>", "motion_viewer.toggle")
    Core.Keybinds.bind("motion_viewer", "<C-q>", "motion_viewer.quit")
end

function MotionViewer.init() end

function MotionViewer.open()
    local doc = nil
    for _, d in ipairs(Cini.documents) do
        local mode = Core.Modes.get_major_mode(d)
        if mode and mode.name == "motion_viewer" then
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

        local view = Cini:create_document_view(doc)
        view.properties["ws"] = nil
        view.properties["nl"] = nil
        view.properties["tab"] = nil

        Cini.workspace.viewport:change_document_view(view)
        MotionViewer.refresh(doc)
    else
        doc = Cini:create_document()
        doc.properties["name"] = "Motion Viewer"
        doc.properties["expanded_motions"] = {}

        local view = Cini:create_document_view(doc)
        view.properties["ws"] = nil
        view.properties["nl"] = nil
        view.properties["tab"] = nil

        Cini.workspace.viewport:change_document_view(view)
        Core.Modes.set_major_mode(doc, "motion_viewer")
        MotionViewer.refresh(doc)
    end
end

--- @param doc Core.Document
function MotionViewer.refresh(doc)
    local major_mode = Core.Modes.get_major_mode(doc)
    if not major_mode or major_mode.name ~= "motion_viewer" then return end

    local views = doc:views()
    local old_rows = {}
    for idx, view in ipairs(views) do
        old_rows[idx] = view.cur.row
        view:move_cursor(function(c, v) c:_jump_to_beginning_of_file(v) end, 0)
    end

    doc:clear()

    local expanded_state = doc.properties["expanded_motions"] or {}

    local motion_names = {}
    for name in pairs(Core.Motions.motions) do table.insert(motion_names, name) end
    table.sort(motion_names)

    local first = true
    for _, name in ipairs(motion_names) do
        local motion = Core.Motions.motions[name]

        local synopsis = (motion.metadata and motion.metadata.synopsis) or ""
        local prefix = expanded_state[name] and "[-] " or "[+] "
        local sequence_text = "(" .. table.concat(motion.sequences, ", ") .. ")"

        local line_text = string.format("%s%s%-20s %-15s  %s",
            first and "" or "\n", prefix, name, sequence_text, synopsis)

        local start = doc.size
        doc:insert(start, line_text)

        -- Plus four or five for skipping [+] or [-] with optional newline.
        local name_start = first and start + 4 or start + 5
        local name_stop = name_start + math.max(20, #name)

        -- Plus one for the space between the name and the sequence.
        local sequence_start = name_stop + 1
        local sequence_stop = sequence_start + math.max(15, #sequence_text)

        doc:add_text_property(start, doc.size, "motion_name", name)
        doc:add_text_property(name_start, name_stop, "face", "motion_viewer.name")
        doc:add_text_property(sequence_start, sequence_stop, "face", "motion_viewer.sequence")
        -- Plus two for double spaces between name and synopsis.
        doc:add_text_property(sequence_stop + 2, doc.size, "face", "motion_viewer.synopsis")

        first = false

        if expanded_state[name] then
            local desc = (motion.metadata and motion.metadata.description) or "No description."

            local desc_start = doc.size
            doc:insert(desc_start, string.format("\n    %s", desc))
            doc:add_text_property(desc_start, doc.size, "face", "motion_viewer.desc")
        end
    end

    doc.modified = false
    for idx, view in ipairs(views) do
        view:move_cursor(Core.Cursor.down, old_rows[idx] or 0)
        MotionViewer.update_selection(view)
    end
end

function MotionViewer.update_selection(view)
    local row = view.cur.row
    local start = view.doc:line_begin_byte(row)
    local stop = view.doc:line_end_byte(row)

    view:clear_view_properties("selection")
    if start ~= stop then
        local line_text = view.doc:slice(start, stop)
        local prefix = line_text:sub(1, 3)

        -- Only highlight on lines that can toggle details.
        if prefix == "[+]" or prefix == "[-]" then
            view:add_view_property(start, stop, "selection", "selection.selection")
        end
    end
end

return MotionViewer
