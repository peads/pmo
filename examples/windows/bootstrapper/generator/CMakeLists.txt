cmake_minimum_required(VERSION 3.29)
project(bootstrapper-generator CXX)
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_C_STANDARD_LIBRARIES "" CACHE STRING "Standard libraries for C" FORCE)
set(CMAKE_CXX_STANDARD_LIBRARIES "" CACHE STRING "Standard libraries for C++" FORCE)

add_executable(bootstrapper-generator ../main.cpp)
target_compile_definitions(bootstrapper-generator PRIVATE
        IS_BOOTSTRAP="${GENERATE_BOOTSTRAP}"
        OUT_PATH="${CMAKE_CURRENT_BINARY_DIR}"
)
target_link_options(bootstrapper-generator PRIVATE "/NODEFAULTLIB" "/MACHINE:X64" "/subsystem:console")
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
            -DCMAKE_C_COMPILER="${CMAKE_C_COMPILER}"
            -DCMAKE_CXX_COMPILER="${CMAKE_CXX_COMPILER}"
            -DCMAKE_CXX_FLAGS="${CMAKE_CXX_FLAGS}"
            -DCMAKE_C_FLAGS="${CMAKE_C_FLAGS}"
            -DGENERATE_TEST="${GENERATE_TEST}"
        COMMAND ${CMAKE_COMMAND} --build "${CMAKE_BINARY_DIR}/bootstrapper"
        COMMENT "Building bootstrapper"
)

if (NOT DEFINED OUT_PATH)
    set(OUT_PATH "${CMAKE_CURRENT_BINARY_DIR}")
endif()
