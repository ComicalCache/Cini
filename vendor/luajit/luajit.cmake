set(LUAJIT_LIB "${CMAKE_BINARY_DIR}/lib/libluajit-5.1.a")
set(LUAJIT_INC "${CMAKE_BINARY_DIR}/include/luajit")
set(BUILD_SCRIPT "${CMAKE_SOURCE_DIR}/vendor/luajit/luajit.sh")

add_custom_command(
        OUTPUT ${LUAJIT_LIB}
        COMMAND bash ${BUILD_SCRIPT} ${CMAKE_BINARY_DIR} "${CMAKE_C_COMPILER}"
        WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
        VERBATIM
)
add_custom_target(luajit_script DEPENDS ${LUAJIT_LIB})
