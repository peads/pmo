cmake_minimum_required(VERSION 3.29)
project(bootstrapper-generator CXX)
set(CMAKE_CXX_STANDARD 23)

#if (NOT DEFINED BOOTSTRAPPER_BUILD_DIR)
#    set(BOOTSTRAPPER_BUILD_DIR "${CMAKE_CURRENT_BINARY_DIR}")
#endif()
add_executable(bootstrapper-generator ../main.cpp)
target_compile_definitions(bootstrapper-generator PRIVATE
        IS_BOOTSTRAP="${GENERATE_BOOTSTRAP}"
        OUT_PATH="${CMAKE_CURRENT_BINARY_DIR}"
)
target_link_options(bootstrapper-generator PRIVATE "/NODEFAULTLIB" "/MACHINE:X64")
#target_link_libraries(bootstrapper-generator PRIVATE "${SW3_BIN_DIR}/syscalls.lib")
target_link_libraries(bootstrapper-generator PRIVATE syscalls)
target_link_libraries(bootstrapper-generator PRIVATE "${LIBS}")

add_custom_command(TARGET bootstrapper-generator
        POST_BUILD
        COMMAND "${CMAKE_CURRENT_BINARY_DIR}/bootstrapper-generator.exe"
        COMMAND ${CMAKE_COMMAND}
            -G "${CMAKE_GENERATOR}"
            -S "${CMAKE_SOURCE_DIR}/bootstrapper"
            -B "${CMAKE_BINARY_DIR}/bootstrapper"
            -DOUT_PATH="${CMAKE_CURRENT_BINARY_DIR}"
            -DPMO_INCLUDE="${PMO_INCLUDE}"
            -DSW3_SRC_DIR="${SW3_SRC_DIR}"
            -DSW3_BIN_DIR="${SW3_BIN_DIR}"
            -DCMAKE_BUILD_TYPE="$<CONFIG>"
            "$<$<NOT:$<CXX_COMPILER_ID:MSVC>>:-DCMAKE_C_COMPILER=\"$<C_COMPILER_ID>\" -DCMAKE_CXX_COMPILER=\"$<CXX_COMPILER_ID>\">"
            -DGENERATE_TEST="${GENERATE_TEST}"
        COMMAND ${CMAKE_COMMAND} --build "${CMAKE_BINARY_DIR}/bootstrapper"
        COMMENT "Building bootstrapper"
)
#add_custom_command(TARGET bootstrapper-generator
#        POST_BUILD
#        COMMAND ${CMAKE_COMMAND} -S "${CMAKE_CURRENT_SOURCE_DIR}/bootstrapper" -B "${CMAKE_CURRENT_BINARY_DIR}/bootstrapper"
#        COMMAND ${CMAKE_COMMAND} --build "${CMAKE_CURRENT_BINARY_DIR}/bootstrapper"
#)
if (NOT DEFINED OUT_PATH)
    set(OUT_PATH "${CMAKE_CURRENT_BINARY_DIR}")
endif()
