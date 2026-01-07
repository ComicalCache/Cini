local M = {}

function M.setup()
    local text = State.editor:get_mode("text")

    M.base("text")
end

function M.base(mode)
    Keybind.bind(mode, "i", function(editor)
        editor.active_viewport.doc:add_minor_mode("insert")
    end)
end

return M
