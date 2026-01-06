local M = {}

function M.setup()
    Keybind.bind("cpp", "i", function(editor)
        editor.active_viewport.doc:add_minor_mode("insert")
    end)

    local cpp = State.editor:get_mode("cpp")

    -- One Dark Pro Theme Colors
    local keyword = Core.Face({ fg = Core.Rgb(198, 120, 221) })
    local type_ = Core.Face({ fg = Core.Rgb(229, 192, 123) })
    local string = Core.Face({ fg = Core.Rgb(152, 195, 121) })
    local number = Core.Face({ fg = Core.Rgb(209, 154, 102) })
    local comment = Core.Face({ fg = Core.Rgb(92, 99, 112) })
    local preprocessor = Core.Face({ fg = Core.Rgb(224, 108, 117) })

    -- Register Faces
    cpp:set_face("cpp:keyword", keyword)
    cpp:set_face("cpp:type", type_)
    cpp:set_face("cpp:string", string)
    cpp:set_face("cpp:number", number)
    cpp:set_face("cpp:comment", comment)
    cpp:set_face("cpp:preprocessor", preprocessor)

    local keywords = {
        "alignas", "alignof", "and", "and_eq", "asm", "atomic_cancel", "atomic_commit",
        "atomic_noexcept", "auto", "bitand", "bitor", "break", "case", "catch",
        "compl", "concept", "const", "const_cast", "continue", "decltype", "default",
        "delete", "do", "dynamic_cast", "else", "enum", "explicit", "export", "extern",
        "for", "friend", "goto", "if", "inline", "mutable", "namespace", "new",
        "noexcept", "not", "not_eq", "nullptr", "operator", "or", "or_eq", "private",
        "protected", "public", "register", "reinterpret_cast", "requires", "return",
        "sizeof", "static", "static_assert", "static_cast", "struct", "switch",
        "synchronized", "template", "this", "thread_local", "throw", "try", "typedef",
        "typeid", "typename", "union", "using", "virtual", "volatile", "while", "xor", "xor_eq"
    }
    local types = {
        "bool", "char", "char16_t", "char32_t", "double", "float", "int",
        "long", "short", "signed", "unsigned", "void", "wchar_t", "size_t"
    }

    -- 1. Keywords & Types
    cpp:set_syntax("\\b(" .. table.concat(keywords, "|") .. ")\\b", "cpp:keyword")
    cpp:set_syntax("\\b(" .. table.concat(types, "|") .. ")\\b", "cpp:type")

    -- 2. Numbers
    cpp:set_syntax("\\b0x[0-9a-fA-F]+\\b", "cpp:number")
    cpp:set_syntax("\\b\\d+(\\.\\d+)?\\b", "cpp:number")

    -- 3. Preprocessor
    cpp:set_syntax("#\\w+", "cpp:preprocessor")

    -- 4. Strings
    cpp:set_syntax("\".*?\"", "cpp:string")

    -- 5. Comments
    cpp:set_syntax("//.*", "cpp:comment")
end

return M
