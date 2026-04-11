local Search = {}

--- @class Search.State
--- @field results table<integer, Core.RegexMatch>
--- @field curr_result integer
local State = {}

function Search.setup()
    -- Faces.
    Core.Faces.register_face("search_match", Core.Face({ fg = Core.Rgb(41, 44, 51), bg = Core.Rgb(229, 192, 123) }))
    Core.Faces.register_face("curr_search_match",
        Core.Face({ fg = Core.Rgb(41, 44, 51), bg = Core.Rgb(235, 168, 45) }))

    -- Modes.
    Core.Modes.register_mode({
        name = "search",
        cursor_style = Core.CursorStyle.SteadyBlock,
    })

    -- Hooks.
    Core.Hooks.add("document::after-insert", 5, function(doc, _, _)
        for _, view in ipairs(doc:views()) do Search.stop(view) end
    end)
    Core.Hooks.add("document::after-remove", 5, function(doc, _, _)
        for _, view in ipairs(doc:views()) do Search.stop(view) end
    end)
    Core.Hooks.add("document::after-clear", 5, function(doc)
        for _, view in ipairs(doc:views()) do Search.stop(view) end
    end)

    -- Commands.
    Core.Commands.register("global.search_file", {
        metadata = {},
        run = function()
            Core.Prompt.run("Search file: ", nil, function(input)
                local view = Cini.workspace.viewport.view
                Search.run(view, input, 0, view.doc.size)
            end)
        end
    })
    Core.Commands.register("global.search_range", {
        metadata = {},
        run = function()
            Core.Prompt.run("Search line range (start,stop): ", nil, function(range_input)
                local start, stop = range_input:match("(%d+)%s*,%s*(%d+)")

                if not start or not stop then
                    Cini:set_status_message("Invalid range.", "error_message", 3000, false)
                    return
                end

                Core.Prompt.run(string.format("Search lines %s-%s: ", start, stop), nil, function(input)
                    local view = Cini.workspace.viewport.view

                    start = math.max(0, tonumber(start) - 1)
                    stop = math.max(0, tonumber(stop) - 1)

                    local max = view.doc:position_from_byte(view.doc.size).row
                    start = math.max(0, math.min(start, max))
                    stop = math.max(0, math.min(stop, max))

                    Search.run(view, input, view.doc:line_begin_byte(start), view.doc:line_end_byte(stop))
                end)
            end)
        end
    })

    Core.Commands.register("search.cancel", {
        metadata = {},
        run = function() Search.stop(Cini.workspace.viewport.view) end
    })
    Core.Commands.register("search.next", {
        metadata = {},
        run = function()
            local view = Cini.workspace.viewport.view
            local state = view.properties["search"]

            if state and #state.results > 0 then
                state.curr_result = (state.curr_result % #state.results) + 1
                Search.update(view)
            end
        end
    })
    Core.Commands.register("search.prev", {
        metadata = {},
        run = function()
            local view = Cini.workspace.viewport.view
            local state = view.properties["search"]

            if state and #state.results > 0 then
                state.curr_result = state.curr_result - 1
                if state.curr_result < 1 then state.curr_result = #state.results end
                Search.update(view)
            end
        end
    })

    -- Keybinds.
    Core.Keybinds.bind("global", "f f", "global.search_file")
    Core.Keybinds.bind("global", "f r", "global.search_range")

    Core.Keybinds.bind("search", "<Esc>", "search.cancel")
    Core.Keybinds.bind("search", "n", "search.next")
    Core.Keybinds.bind("search", "N", "search.prev")
end

function Search.init() end

--- @param view Core.DocumentView
--- @param pattern string
--- @param start integer
--- @param stop integer
function Search.run(view, pattern, start, stop)
    if not pattern or pattern == "" then return end

    local regex, err = Core.Regex(pattern)
    if not regex then
        Cini:set_status_message(("Regex error: %s"):format(err), "error_message", 0, false)
        return
    end

    local matches = view.doc:search(regex, start, stop)
    if not matches or #matches == 0 then
        Cini:set_status_message("No matches found", "info_message", 3000, false)
        Search.stop(view)
        return
    end

    view.properties["search"] = {
        results = matches,
        curr_result = 1
    }

    Core.Modes.add_minor_mode(view, "search")
    Search.update(view)

    Cini:set_status_message(("Found %d matches"):format(#matches), "info_message", 3000, false)
end

--- @param view Core.DocumentView
function Search.stop(view)
    if not view.properties["search"] then return end

    view.properties["search"] = nil
    Core.Modes.remove_minor_mode(view, "search")
    view:clear_view_properties("search")
end

--- @param view Core.DocumentView
function Search.update(view)
    view:clear_view_properties("search")

    local state = view.properties["search"]
    --- @cast state Search.State?
    if not state then return end

    for idx, match in ipairs(state.results) do
        local face = (idx == state.curr_result) and "curr_search_match" or "search_match"
        view:add_view_property(match.start, match.stop, "search", face)
    end

    local active_match = state.results[state.curr_result]
    if active_match then view:move_cursor(function(c, v, _) c:move_to(v, active_match.start) end, 0) end
end

return Search
