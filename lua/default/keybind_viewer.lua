local KeybindViewer = {}

function KeybindViewer.setup()
    -- Faces.
    Core.Faces.register_face("keybind_viewer.mode", Core.Face({ fg = Core.Rgb(97, 175, 239), bold = true }))
    Core.Faces.register_face("keybind_viewer.sequence", Core.Face({ fg = Core.Rgb(235, 145, 70) }))
    Core.Faces.register_face("keybind_viewer.cmd", Core.Face({ fg = Core.Rgb(97, 175, 239) }))
    Core.Faces.register_face("keybind_viewer.synopsis", Core.Face({ fg = Core.Rgb(126, 128, 130), italic = true }))

    -- Mode.
    local current_line_override = Core.Faces.get_face("default") or {}
    Core.Modes.register_mode({
        name = "keybind_viewer",
        faces = { current_line = Core.Face({ bg = current_line_override.bg }) },
        mode_line_layout = {
            { callback = function(_) return { { text = "Keybind Viewer" } } end },
            "minor_mode_indicators",
            "pending_keys",
            "spacer",
            "cursor_pos",
        },
        metadata = {
            read_only = true
        }
    })

    -- Commands.
    Core.Commands.register("global.keybind_viewer", {
        metadata = {
            synopsis = "Open the keybind viewer",
            description = "Opens a buffer listing all keybinds by mode.",
        },
        callback = function() KeybindViewer.open() end
    })

    Core.Commands.register("keybind_viewer.quit", {
        metadata = {
            synopsis = "Exits the keybind viewer",
            description = "Exits the keybind viewer and closes the buffer.",
        },
        callback = function() Cini:destroy_document(Cini.workspace.viewport.view.doc) end
    })

    -- Keybinds.
    Core.Keybinds.bind("global", "<M-k>", "global.keybind_viewer")

    Core.Keybinds.bind("keybind_viewer", "<C-q>", "keybind_viewer.quit")
end

function KeybindViewer.init() end

function KeybindViewer.open()
    local doc = nil
    for _, d in ipairs(Cini.documents) do
        local mode = Core.Modes.get_major_mode(d)
        if mode and mode.name == "keybind_viewer" then
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
        KeybindViewer.refresh(doc)
    else
        doc = Cini:create_document()
        doc.properties["name"] = "Keybind Viewer"

        Cini.workspace.viewport:change_document_view(Cini:create_document_view(doc))
        Core.Modes.set_major_mode(doc, "keybind_viewer")
        KeybindViewer.refresh(doc)
    end
end

--- @param doc Core.Document
function KeybindViewer.refresh(doc)
    local major_mode = Core.Modes.get_major_mode(doc)
    if not major_mode or major_mode.name ~= "keybind_viewer" then return end

    doc:clear()

    local mode_names = {}
    for name in pairs(Core.Modes.modes) do table.insert(mode_names, name) end
    table.sort(mode_names)

    local first = true
    for _, mode_name in ipairs(mode_names) do
        local mode = Core.Modes.modes[mode_name]
        if mode.keymap then
            --- @type { sequence: string, command: string }[]
            local binds = {}

            --- @param map table<string, string|table>
            --- @param prefix string
            local function traverse(map, prefix)
                for k, v in pairs(map) do
                    local seq = prefix == "" and k or (prefix .. " " .. k)
                    if type(v) == "table" then
                        traverse(v, seq)
                    elseif type(v) == "string" then
                        table.insert(binds, { sequence = seq, command = v })
                    end
                end
            end

            if mode.keymap then
                traverse(mode.keymap, "")

                if #binds > 0 then
                    table.sort(binds, function(a, b) return a.sequence < b.sequence end)

                    local mode_start = doc.size
                    doc:insert(mode_start, string.format("%s[%s]", first and "" or "\n\n", mode_name:upper()))
                    doc:add_text_property(mode_start, doc.size, "face", "keybind_viewer.mode")
                    first = false

                    for _, bind in ipairs(binds) do
                        local cmd = Core.Commands.registry[bind.command]
                        local synopsis = (cmd and cmd.metadata and cmd.metadata.synopsis) or ""
                        if synopsis ~= "" then synopsis = "  " .. synopsis end

                        local bind_start = doc.size
                        local text = string.format("\n    %-15s %s", bind.sequence, bind.command)
                        doc:insert(bind_start, text)

                        local seq_stop = bind_start + 5 + math.max(15, #bind.sequence)
                        doc:add_text_property(bind_start + 5, seq_stop, "face", "keybind_viewer.sequence")

                        local cmd_stop = doc.size
                        doc:add_text_property(seq_stop, cmd_stop, "face", "keybind_viewer.cmd")

                        if synopsis ~= "" then
                            local synopsis_start = doc.size
                            doc:insert(synopsis_start, synopsis)
                            doc:add_text_property(synopsis_start, doc.size, "face", "keybind_viewer.synopsis")
                        end
                    end
                end
            end
        end
    end

    doc.modified = false
    for _, view in ipairs(doc:views()) do
        view:move_cursor(function(c, v) c:_jump_to_beginning_of_file(v) end, 0)
    end
end

return KeybindViewer
