set(INPUT_FILE ${CMAKE_ARGV3})
set(OUTPUT_FILE ${CMAKE_ARGV4})

file(READ ${INPUT_FILE} HEX_CONTENT HEX)
string(REGEX REPLACE "(..)" "0x\\1, " HEX_CONTENT "${HEX_CONTENT}")

file(WRITE ${OUTPUT_FILE} "// Auto-generated from defaults.lua.\n\n")
file(APPEND ${OUTPUT_FILE} "#include <cstddef>\n\n")
file(APPEND ${OUTPUT_FILE} "namespace lua_defaults {\n")
file(APPEND ${OUTPUT_FILE} "    /// Binary data of the defaults.lua file.\n")
file(APPEND ${OUTPUT_FILE} "    const unsigned char data[] = { ${HEX_CONTENT} 0x00 };\n")
file(APPEND ${OUTPUT_FILE} "    /// Length of the binary data.\n")
file(APPEND ${OUTPUT_FILE} "    const std::size_t len = sizeof(data) - 1;\n")
file(APPEND ${OUTPUT_FILE} "}\n")
