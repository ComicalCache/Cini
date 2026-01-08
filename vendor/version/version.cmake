set(VERSION_SOURCE "${CMAKE_BINARY_DIR}/version/version.cpp")

add_custom_target(version
        COMMAND ${CMAKE_COMMAND}
        -DPROJECT_NAME="${PROJECT_NAME}"
        -DPROJECT_VERSION="${PROJECT_VERSION}"
        -DPROJECT_BUILD_TYPE="${CMAKE_BUILD_TYPE}"
        -DVERSION_SOURCE="${VERSION_SOURCE}"
        -DVERSION_TEMPLATE="${CMAKE_CURRENT_SOURCE_DIR}/gen/version.cpp.in"
        -DSRC="${CMAKE_CURRENT_SOURCE_DIR}"
        -P "${CMAKE_SOURCE_DIR}/vendor/version/generate.cmake"
        BYPRODUCTS "${VERSION_SOURCE}"
        COMMENT "Embedding version information..."
)
