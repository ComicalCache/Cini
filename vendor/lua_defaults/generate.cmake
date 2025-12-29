set(INPUT_DIR ${CMAKE_ARGV3})
set(OUTPUT_FILE ${CMAKE_ARGV4})

set(CONTENT "// Auto-generated from ${INPUT_DIR}.\n\n")
set(CONTENT "${CONTENT}#ifndef LUA_DEFAULTS_HPP_\n#define LUA_DEFAULTS_HPP_\n\n#include <map>\n#include <string>\n\n")
set(CONTENT "${CONTENT}namespace lua_modules {\n")
set(CONTENT "${CONTENT}    const std::map<std::string, std::string> files = {\n")

# Get all lua files.
file(GLOB_RECURSE FILES RELATIVE "${INPUT_DIR}" "${INPUT_DIR}/*.lua")

foreach(FILE ${FILES})
    file(READ "${INPUT_DIR}/${FILE}" HEX_CONTENT HEX)
    string(REGEX REPLACE "(..)" "static_cast<char>(0x\\1), " HEX_FORMATTED "${HEX_CONTENT}")

    # Calculate module name.
    string(REPLACE "/" "." MOD_NAME "${FILE}")
    string(REPLACE ".lua" "" MOD_NAME "${MOD_NAME}")

    # Build map.
    set(CONTENT "${CONTENT}        {\"${MOD_NAME}\", { ${HEX_FORMATTED} }},\n")
endforeach()

set(CONTENT "${CONTENT}    };\n}\n\n#endif\n")

# Write to file on changes.
if(EXISTS "${OUTPUT_FILE}")
    file(READ "${OUTPUT_FILE}" OLD_CONTENT)
endif()
if(NOT "${CONTENT}" STREQUAL "${OLD_CONTENT}")
    file(WRITE "${OUTPUT_FILE}" "${CONTENT}")
endif()