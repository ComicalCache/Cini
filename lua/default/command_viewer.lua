local CommandViewer = {}

function CommandViewer.setup()
    -- Faces.
    Core.Faces.register_face("command_viewer.name", Core.Face({ fg = Core.Rgb(97, 175, 239), bold = true }))
    Core.Faces.register_face("command_viewer.synopsis", Core.Face({ fg = Core.Rgb(126, 128, 130), italic = true }))
    Core.Faces.register_face("command_viewer.desc", Core.Face({ fg = Core.Rgb(172, 178, 190) }))
    Core.Faces.register_face("command_viewer.keybind", Core.Face({ fg = Core.Rgb(235, 145, 70) }))

    -- Mode.
    local current_line_override = Core.Faces.get_face("default") or {}
    Core.Modes.register_mode({
        name = "command_viewer",
        faces = { current_line = Core.Face({ bg = current_line_override.bg }) },
        mode_line_layout = {
            { run = function(_) return { { text = "Command Viewer" } } end },
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
        return not (legal and mode and mode.name == "command_viewer")
    end)

    Core.Hooks.add("cursor::after-move", 50, function(view, _)
        local mode = Core.Modes.get_major_mode(view.doc)
        if mode and mode.name == "command_viewer" then CommandViewer.update_selection(view) end
    end)

    Core.Hooks.add("document::set-major-mode", 50, function(doc, mode)
        --- @cast doc Core.Document
        --- @cast mode string

        if mode ~= "command_viewer" then return end

        for _, view in ipairs(doc:views()) do
            view.properties["ws"] = nil
            view.properties["nl"] = nil
            view.properties["tab"] = nil
        end
    end)

    Core.Hooks.add("document_view::created", 50, function(view)
        --- @cast view Core.DocumentView

        local mode = Core.Modes.get_major_mode(view.doc)

        if mode and mode.name == "command_viewer" then
            view.properties["ws"] = nil
            view.properties["nl"] = nil
            view.properties["tab"] = nil
        end
    end)

    -- Commands.
    Core.Commands.register("global.command_viewer", {
        metadata = {
            synopsis = "Open the command viewer",
            description = "Opens a buffer listing all registered commands and their descriptions."
        },
        run = function() CommandViewer.open() end
    })

    Core.Commands.register("command_viewer.toggle", {
        metadata = {
            synopsis = "Toggles the details of a command",
            description = "Toggles more details of a command like a full description or associated keybinds.",
        },
        run = function()
            local view = Cini.workspace.viewport.view
            local cmd_name = view.doc:get_text_property(view.cur:point(view), "command_name")
            if not cmd_name then return end

            local expanded = view.doc.properties["expanded_commands"] or {}
            expanded[cmd_name] = not expanded[cmd_name]
            view.doc.properties["expanded_commands"] = expanded

            CommandViewer.refresh(view.doc)
        end
    })

    Core.Commands.register("command_viewer.quit", {
        metadata = {
            synopsis = "Exits the command viewer",
            description = "Exits the command viewer, closing the buffer.",
        },
        run = function() Cini:destroy_document(Cini.workspace.viewport.view.doc) end
    })

    -- Keybinds
    Core.Keybinds.bind("global", "<M-c>", "global.command_viewer")

    Core.Keybinds.bind("command_viewer", "<Enter>", "command_viewer.toggle")
    Core.Keybinds.bind("command_viewer", "<C-q>", "command_viewer.quit")
end

function CommandViewer.init() end

function CommandViewer.open()
    local doc = nil
    for _, d in ipairs(Cini.documents) do
        local mode = Core.Modes.get_major_mode(d)
        if mode and mode.name == "command_viewer" then
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
        CommandViewer.refresh(doc)
    else
        doc = Cini:create_document()
        doc.properties["name"] = "Command Viewer"
        doc.properties["expanded_commands"] = {}

        local view = Cini:create_document_view(doc)
        view.properties["ws"] = nil
        view.properties["nl"] = nil
        view.properties["tab"] = nil

        Cini.workspace.viewport:change_document_view(view)
        Core.Modes.set_major_mode(doc, "command_viewer")
        CommandViewer.refresh(doc)
    end
end

local function get_command_keybinds(target_cmd)
    local results = {}

    for mode_name, mode in pairs(Core.Modes.modes) do
        if mode.keymap then
            --- @param map table<string, string|table>
            --- @param prefix string
            local function traverse(map, prefix)
                for k, v in pairs(map) do
                    local seq = prefix == "" and k or (prefix .. " " .. k)
                    if type(v) == "table" then
                        traverse(v, seq)
                    elseif type(v) == "string" and v == target_cmd then
                        table.insert(results, { mode = mode_name, sequence = seq })
                    end
                end
            end

            if mode.keymap then traverse(mode.keymap, "") end
        end
    end

    return results
end

--- @param doc Core.Document
function CommandViewer.refresh(doc)
    local major_mode = Core.Modes.get_major_mode(doc)
    if not major_mode or major_mode.name ~= "command_viewer" then return end

    local views = doc:views()
    local old_rows = {}
    for idx, view in ipairs(views) do
        old_rows[idx] = view.cur.row
        view:move_cursor(function(c, v) c:_jump_to_beginning_of_file(v) end, 0)
    end

    doc:clear()

    local expanded_state = doc.properties["expanded_commands"] or {}

    local cmd_names = {}
    for name in pairs(Core.Commands.registry) do table.insert(cmd_names, name) end
    table.sort(cmd_names)

    local first = true
    for _, name in ipairs(cmd_names) do
        local cmd = Core.Commands.registry[name]

        local synopsis = (cmd.metadata and cmd.metadata.synopsis) or ""
        local prefix = expanded_state[name] and "[-] " or "[+] "
        local line_text = string.format("%s%s%s  %s", first and "" or "\n", prefix, name, synopsis)

        local start = doc.size
        doc:insert(start, line_text)

        -- Plus four or five for skipping [+] or [-] with optional newline.
        local name_start = first and start + 4 or start + 5
        local name_stop = name_start + #name
        doc:add_text_property(start, doc.size, "command_name", name)
        doc:add_text_property(name_start, name_stop, "face", "command_viewer.name")
        -- Plus two for double spaces between name and synopsis.
        doc:add_text_property(name_stop + 2, doc.size, "face", "command_viewer.synopsis")

        first = false

        if expanded_state[name] then
            local desc = (cmd.metadata and cmd.metadata.description) or "No description"
            local desc_text = string.format("\n    Description: %s", desc)

            local desc_start = doc.size
            doc:insert(desc_start, desc_text)
            doc:add_text_property(desc_start, doc.size, "face", "command_viewer.desc")

            local binds = get_command_keybinds(name)
            if #binds > 0 then
                doc:insert(doc.size, "\n    Bound to:")

                for _, b in ipairs(binds) do
                    local binding_start = doc.size

                    doc:insert(binding_start, string.format("\n        %s: %s", b.mode, b.sequence))
                    doc:add_text_property(binding_start, doc.size, "face", "command_viewer.keybind")
                end
            end
        end
    end

    doc.modified = false
    for idx, view in ipairs(views) do
        view:move_cursor(Core.Cursor.down, old_rows[idx] or 0)
        CommandViewer.update_selection(view)
    end
end

function CommandViewer.update_selection(view)
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

return CommandViewer
