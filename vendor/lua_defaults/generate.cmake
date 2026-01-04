set(FILE_MAP "{\n")

# Get all lua files.
file(GLOB_RECURSE FILES RELATIVE "${INPUT_DIR}" "${INPUT_DIR}/*.lua")

foreach (FILE ${FILES})
    file(READ "${INPUT_DIR}/${FILE}" HEX_CONTENT HEX)
    string(REGEX REPLACE "(..)" "static_cast<char>(0x\\1), " HEX_FORMATTED "${HEX_CONTENT}")

    # Calculate module name.
    string(REPLACE "/" "." MOD_NAME "${FILE}")
    string(REPLACE ".lua" "" MOD_NAME "${MOD_NAME}")

    set(FILE_MAP "${FILE_MAP}        {\"${MOD_NAME}\", { ${HEX_FORMATTED} }},\n")
endforeach ()

set(FILE_MAP "${FILE_MAP}    }")

configure_file("${LUA_DEFAULTS_TEMPLATE}" "${LUA_DEFAULTS_SOURCE}" @ONLY)
