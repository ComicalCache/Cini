local M = {}

function M.setup()
    text = State.editor:get_mode("text")

    Keybind.bind("text", "i", function(editor)
        editor.active_viewport.doc:add_minor_mode("insert")
    end)
end

return M
