cmake_minimum_required(VERSION 4.2.1)
project(
        Cini
        VERSION 1.0.0
        LANGUAGES C CXX
)

if (NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)
    set(PROJECT_BUILD_TYPE "release")
else ()
    set(PROJECT_BUILD_TYPE "debug")
endif ()

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin")
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib")

# Vendor needs to be added prior to src as src tries to perform LTO.
add_subdirectory(vendor)
add_subdirectory(src)
