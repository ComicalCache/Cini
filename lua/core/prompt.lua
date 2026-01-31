--- @class Core.Prompt
local Prompt = {}

Prompt.active = false
--- @type fun(input: string)|nil
Prompt.callback = nil
Prompt.prefix_len = 0

function Prompt.init()
    Core.Prompt = Prompt

    Core.Modes.register_mode("prompt", Core.Mode.new({
        name = "prompt",
        keymap = {
            ["<Enter>"] = Prompt.submit,
            ["<Esc>"] = Prompt.cancel,
            ["<Bspc>"] = Prompt.backspace,
            ["<Left>"] = Prompt.left
        }
    }))
end

--- Opens the mini-buffer with a prompt.
--- @param text string The prompt.
--- @param default string? Default value.
--- @param callback fun(input: string) Called with the user input.
function Prompt.run(text, default, callback)
    local doc = State.editor.workspace.mini_buffer.doc

    Prompt.active = true
    Prompt.callback = callback
    Prompt.prefix_len = Core.Utf8.count(text)

    default = default or ""

    State.editor.workspace:enter_mini_buffer()
    doc:clear()
    doc:insert(0, text .. default)

    doc:add_text_property(0, #text, "face", Core.Face({ fg = Core.Rgb(97, 175, 239) }))

    --- Disable inputs on the prompt.
    doc:add_text_property(0, #text, "keymap", {
        ["<CatchAll>"] = function()
            State.editor.workspace.mini_buffer:move_cursor(function(c, d)
                c:move_to(d, Prompt.prefix_len)
            end, 0)

            return false
        end
    })

    Core.Modes.add_minor_mode(doc, "prompt")
    State.editor.workspace.mini_buffer:move_cursor(function(c, d, _) c:_jump_to_end_of_file(d) end, 0)
end

function Prompt.submit()
    if not Prompt.active then return end

    local doc = State.editor.workspace.mini_buffer.doc

    local full_line = doc:line(0)
    local input = string.sub(full_line, Prompt.prefix_len + 1)

    local callback = Prompt.callback

    Prompt.cleanup()

    if callback then
        callback(input)
    end
end

function Prompt.cancel()
    Prompt.cleanup()
end

function Prompt.cleanup()
    Prompt.active = false
    Prompt.callback = nil
    Prompt.prefix_len = 0

    local doc = State.editor.workspace.mini_buffer.doc
    Core.Modes.remove_minor_mode(doc, "prompt")

    doc:clear()

    State.editor.workspace:exit_mini_buffer()
end

--- Prevents deleting the prompt.
function Prompt.backspace()
    local doc = State.editor.workspace.mini_buffer.doc

    if doc.point > Prompt.prefix_len then
        State.editor.workspace.mini_buffer:move_cursor(Core.Cursor.left, 1)
        doc:remove(doc.point, doc.point + 1)
    end
end

--- Prevents moving into the prompt.
function Prompt.left()
    local doc = State.editor.workspace.mini_buffer.doc

    if doc.point > Prompt.prefix_len then
        State.editor.workspace.mini_buffer:move_cursor(Core.Cursor.left, 1)
    end
end

return Prompt
