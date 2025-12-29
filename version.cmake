set(VERSION_TEMPLATE "${CMAKE_CURRENT_SOURCE_DIR}/version.hpp.in")
set(VERSION_HEADER "${CMAKE_BINARY_DIR}/version/version.hpp")
set(VERSION_SCRIPT "${CMAKE_BINARY_DIR}/scripts/version_gen.cmake")

file(WRITE "${VERSION_SCRIPT}" "
    # Calculate the timestamp (runs every build)
    string(TIMESTAMP PROJECT_BUILD_DATE \"%d/%m/%Y - %H:%M:%S\")

    # Set variables expected by version.hpp.in
    set(PROJECT_NAME \"${PROJECT_NAME}\")
    set(PROJECT_VERSION \"${PROJECT_VERSION}\")
    set(PROJECT_BUILD_TYPE \"${CMAKE_BUILD_TYPE}\")

    # Generate the header
    configure_file(\"${VERSION_TEMPLATE}\" \"${VERSION_HEADER}\" @ONLY)
")

# 3. Create the target that runs the script
add_custom_target(version
        COMMAND ${CMAKE_COMMAND} -P "${VERSION_SCRIPT}"
        BYPRODUCTS "${VERSION_HEADER}"
        COMMENT "Embedding version information into a C++ header..."
        VERBATIM
)
