local Replace = {}

function Replace.setup()
    -- Commands.
    Core.Commands.register("global.replace_file", {
        metadata = { modifies = true },
        run = function()
            Core.Prompt.run("Replace (regex): ", nil, function(pattern)
                if not pattern or pattern == "" then return end

                Core.Prompt.run("With: ", nil, function(replacement)
                    local view = Cini.workspace.viewport.view
                    Replace.run(view, pattern, replacement, 0, view.doc.size)
                end)
            end)
        end
    })
    Core.Commands.register("global.replace_range", {
        metadata = { modifies = true },
        run = function()
            Core.Prompt.run("Replace line range (start,stop): ", nil, function(range_input)
                local start, stop = range_input:match("(%d+)%s*,%s*(%d+)")

                if not start or not stop then
                    Cini:set_status_message("Invalid range.", "error_message", 3000, false)
                    return
                end

                Core.Prompt.run(string.format("Replace lines %s-%s (regex): ", start, stop), nil, function(pattern)
                    if not pattern or pattern == "" then return end

                    Core.Prompt.run("With: ", nil, function(replacement)
                        local view = Cini.workspace.viewport.view

                        start = math.max(0, tonumber(start) - 1)
                        stop = math.max(0, tonumber(stop) - 1)

                        local max = view.doc:position_from_byte(view.doc.size).row
                        start = math.max(0, math.min(start, max))
                        stop = math.max(0, math.min(stop, max))

                        Replace.run(view, pattern, replacement, view.doc:line_begin_byte(start),
                            view.doc:line_end_byte(stop))
                    end)
                end)
            end)
        end
    })

    -- Keybinds.
    Core.Keybinds.bind("global", "r f", "global.replace_file")
    Core.Keybinds.bind("global", "r r", "global.replace_range")
end

function Replace.init() end

--- @param view Core.DocumentView
--- @param pattern string
--- @param replacement string
--- @param start integer
--- @param stop integer
function Replace.run(view, pattern, replacement, start, stop)
    if not pattern or pattern == "" then return end

    local regex, err = Core.Regex(pattern)
    if not regex then
        Cini:set_status_message(("Regex error: %s"):format(err), "error_message", 0, false)
        return
    end

    local matches = view.doc:search(regex, start, stop)
    if not matches or #matches == 0 then
        Cini:set_status_message("No matches found", "info_message", 3000, false)
        return
    end

    local pos = view.cur:point(view)
    view.doc:begin_transaction(pos)
    for idx = #matches, 1, -1 do
        local match = matches[idx]
        view.doc:replace(match.start, match.stop, replacement)
    end
    view.doc:end_transaction(pos)

    Cini:set_status_message(("Replaced %d occurrences"):format(#matches), "info_message", 3000, false)
end

return Replace
