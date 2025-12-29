-- global mode
global = State.editor:get_mode("global")

Keybind.bind("global", "<C-q>", function(editor)
    editor:quit()
end)

Keybind.bind("global", "<C-g>", function(editor)
    editor.active_viewport:toggle_gutter()
end)

Keybind.bind("global", "h", function(editor)
    editor.active_viewport:cursor_left(1)
end)
Keybind.bind("global", "j", function(editor)
    editor.active_viewport:cursor_down(1)
end)
Keybind.bind("global", "k", function(editor)
    editor.active_viewport:cursor_up(1)
end)
Keybind.bind("global", "l", function(editor)
    editor.active_viewport:cursor_right(1)
end)

Keybind.bind("global", "<S-h>", function(editor)
    editor.active_viewport:scroll_left(1)
end)
Keybind.bind("global", "<S-j>", function(editor)
    editor.active_viewport:scroll_down(1)
end)
Keybind.bind("global", "<S-k>", function(editor)
    editor.active_viewport:scroll_up(1)
end)
Keybind.bind("global", "<S-l>", function(editor)
    editor.active_viewport:scroll_right(1)
end)

Keybind.bind("global", "i", function(editor)
    editor.active_viewport.doc:add_minor_mode("insert")
end)

default = Core.Face({ fg = Core.Rgb(200, 200, 200), bg = Core.Rgb(50, 50, 50) })
gutter = Core.Face({ fg = Core.Rgb(150, 150, 150), bg = Core.Rgb(0, 0, 0) })
tab = Core.Face({ fg = Core.Rgb(255, 0, 0), bg = Core.Rgb(100, 0, 0) })
whitespace = Core.Face({ fg = Core.Rgb(150, 150, 150), bg = Core.Rgb(50, 50, 50) })

global:set_face("default", default)
global:set_face("gutter", gutter)
global:set_face("tab", tab)
global:set_face("whitespace", whitespace)
global:set_face("error", tab)

global:set_replacement("\t", "↦", "tab")
global:set_replacement(" ", "·", "whitespace")
global:set_replacement("\r", "↤", "whitespace")
global:set_replacement("\n", "⏎", "whitespace")
global:set_replacement("�", "�", "err")

-- insert mode
insert = State.editor:get_mode("insert")

Keybind.bind("insert", "<Esc>", function(editor)
    editor.active_viewport.doc:remove_minor_mode("insert")
end)

Keybind.bind("insert", "<Left>", function(editor)
    editor.active_viewport:cursor_left(1)
end)
Keybind.bind("insert", "<Down>", function(editor)
    editor.active_viewport:cursor_down(1)
end)
Keybind.bind("insert", "<Up>", function(editor)
    editor.active_viewport:cursor_up(1)
end)
Keybind.bind("insert", "<Right>", function(editor)
    editor.active_viewport:cursor_right(1)
end)

insert:bind_catch_all(function(editor, key)
    local doc = editor.active_viewport.doc
    local text = key:to_string()
    doc:insert(editor.active_viewport.cursor:byte(doc), text)
    editor.active_viewport:cursor_right(#text)

    return true
end)

