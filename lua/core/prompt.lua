-- The prompt is the rare case of permitted global state. In Cini there can only ever be one prompt active, thus it
-- is unnecessary to attach it to a DocumentView or the such. This is the exception! Most cases want to attach their
-- state to either the DocumentView or Document itself.

--- @class Core.Prompt
local Prompt = {}

Prompt.active = false
--- @type fun(input: string)|nil
Prompt.callback = nil
Prompt.prefix_len = 0
Prompt.raw_prefix_len = 0

function Prompt.init()
    -- Modes.
    Core.Modes.register_mode({
        name = "prompt",
        cursor_style = Core.CursorStyle.BlinkingBar
    })

    -- Hooks.
    Core.Hooks.add("cursor::before-move", 1, function(view, point)
        --- @cast view Core.DocumentView
        --- @cast point integer

        if not Core.Modes.has_minor_mode(view, "prompt") then return true end
        return point >= Prompt.raw_prefix_len
    end)

    -- Commands.
    Core.Commands.register("prompt.submit", {
        metadata = {},
        run = Prompt.submit
    })
    Core.Commands.register("prompt.cancel", {
        metadata = {},
        run = Prompt.cancel
    })
    Core.Commands.register("prompt.prevent_prompt_edit", {
        metadata = {},
        run = function()
            Cini.workspace.mini_buffer.view:move_cursor(function(c, v, _) c:move_to(v, Prompt.raw_prefix_len) end, 0)
            return true
        end
    })

    -- Keybinds.
    Core.Keybinds.bind("prompt", "<Enter>", "prompt.submit")
    Core.Keybinds.bind("prompt", "<Esc>", "prompt.cancel")

    Core.Prompt = Prompt
end

--- Opens the Mini Buffer with a prompt.
--- @param text string The prompt.
--- @param default string? Default value.
--- @param callback fun(input: string) Called with the user input.
function Prompt.run(text, default, callback)
    local view = Cini.workspace.mini_buffer.view
    local doc = view.doc

    Prompt.active = true
    Prompt.callback = callback
    Prompt.prefix_len = Core.Utf8.count(text)
    Prompt.raw_prefix_len = #text

    default = default or ""

    Cini.workspace:enter_mini_buffer()
    doc:clear()
    doc:insert(0, text .. default)

    doc:add_text_property(0, #text, "face", "info_message")

    --- Disable inputs on the prompt.
    doc:add_text_property(0, #text, "keymap", { ["<CatchAll>"] = "prompt.prevent_prompt_edit" })

    Core.Modes.add_minor_mode(view, "prompt")
    Cini.workspace.mini_buffer.view:move_cursor(function(c, v, _) c:_jump_to_end_of_file(v) end, 0)
end

--- Submits the current prompt input on <Enter>.
function Prompt.submit()
    if not Prompt.active then return end

    local doc = Cini.workspace.mini_buffer.view.doc

    local full_line = doc:line(0)
    local input = string.sub(full_line, Prompt.prefix_len + 1)

    local callback = Prompt.callback

    Prompt.cleanup()

    if callback then callback(input) end
end

--- Cancels the current prompt on <Esc>.
function Prompt.cancel()
    Prompt.cleanup()
end

--- Cleans up prompt state and exits the Mini Buffer.
function Prompt.cleanup()
    Prompt.active = false
    Prompt.callback = nil
    Prompt.prefix_len = 0

    local view = Cini.workspace.mini_buffer.view
    Core.Modes.remove_minor_mode(view, "prompt")

    view.doc:clear()

    Cini.workspace:exit_mini_buffer()
end

return Prompt
