cmake_minimum_required(VERSION 3.29)
project(bootstrapper-generator CXX)
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_C_STANDARD_LIBRARIES "" CACHE STRING "Standard libraries for C" FORCE)
set(CMAKE_CXX_STANDARD_LIBRARIES "" CACHE STRING "Standard libraries for C++" FORCE)

string(TOUPPER "${CMAKE_BUILD_TYPE}" uppercase_CMAKE_BUILD_TYPE)
if (uppercase_CMAKE_BUILD_TYPE STREQUAL "DEBUG")
    set(LIBS "libvcruntimed.lib" "libcmtd.lib" "ucrtd.lib" "kernel32.lib" "msvcprtd.lib")
else()
    set(LIBS "libvcruntime.lib" "libcmt.lib" "ucrt.lib" "kernel32.lib" "msvcprt.lib")
    add_compile_definitions(OPTIMIZATIONS_ON)
endif()

add_executable(bootstrapper-generator "${CMAKE_CURRENT_SOURCE_DIR}/main.cpp")
target_include_directories(bootstrapper-generator PRIVATE "${PMO_INCLUDE}" "${SW3_SRC_DIR}" "${CMAKE_CURRENT_SOURCE_DIR}/../include")
target_compile_definitions(bootstrapper-generator PRIVATE IS_BOOTSTRAP=ON OUT_PATH="${CMAKE_CURRENT_BINARY_DIR}")

target_link_libraries(bootstrapper-generator PRIVATE syscalls)
target_link_libraries(bootstrapper-generator PRIVATE "${LIBS}")
target_link_options(bootstrapper-generator PRIVATE "/NODEFAULTLIB" "/MACHINE:X64" "/subsystem:console")

file(WRITE "${CMAKE_BINARY_DIR}/bootstrapper/.touch")
if (DEFINED DLL_NAME)
    add_custom_command(TARGET bootstrapper-generator
            POST_BUILD
            COMMAND "$<TARGET_FILE:bootstrapper-generator>" "${DLL_NAME}"
            COMMAND ${CMAKE_COMMAND}
                -G "${CMAKE_GENERATOR}"
                -S "${CMAKE_SOURCE_DIR}/bootstrapper"
                -B "${CMAKE_BINARY_DIR}/bootstrapper"
                -DOUT_PATH="${CMAKE_CURRENT_BINARY_DIR}"
                -DPMO_INCLUDE="${PMO_INCLUDE}"
                -DSW3_SRC_DIR="${SW3_SRC_DIR}"
                -DSW3_BIN_DIR="${SW3_BIN_DIR}"
                -DCMAKE_BUILD_TYPE="${CMAKE_BUILD_TYPE}"
                -DCMAKE_C_COMPILER="${CMAKE_C_COMPILER}"
                -DCMAKE_CXX_COMPILER="${CMAKE_CXX_COMPILER}"
                -DCMAKE_CXX_FLAGS="${CMAKE_CXX_FLAGS}"
                -DCMAKE_C_FLAGS="${CMAKE_C_FLAGS}"
                -DGENERATE_TEST="${GENERATE_TEST}"
                -DDLL_NAME="${DLL_NAME}"
            COMMAND ${CMAKE_COMMAND} --build "${CMAKE_BINARY_DIR}/bootstrapper"
            COMMENT "Building bootstrapper"
    )
endif()

if (NOT DEFINED OUT_PATH)
    set(OUT_PATH "${CMAKE_CURRENT_BINARY_DIR}")
endif()

set_target_properties(bootstrapper-generator PROPERTIES MSVC_RUNTIME_CHECKS "")
