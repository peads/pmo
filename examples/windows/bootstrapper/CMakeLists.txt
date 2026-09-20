cmake_minimum_required(VERSION 3.29)
project(Example4)

find_package(Git REQUIRED)
include(FetchContent)
set(SW3_SRC_DIR "${CMAKE_CURRENT_BINARY_DIR}/_deps/syswhispers3-src")
set(SW3_BIN_DIR "${CMAKE_CURRENT_BINARY_DIR}/_deps/syswhispers3-build")
set(SW3_SUB_DIR "${CMAKE_CURRENT_BINARY_DIR}/_deps/syswhispers3-subbuild")

FetchContent_Populate(
        syswhispers3
        QUIET
        GIT_REPOSITORY https://github.com/klezVirus/SysWhispers3.git
        SOURCE_DIR ${SW3_SRC_DIR}
        BINARY_DIR ${SW3_BIN_DIR}
        SUBBUILD_DIR ${SW3_SUB_DIR}
)

configure_file(
    "${CMAKE_CURRENT_SOURCE_DIR}/syswhispers3.cmake"
    "${SW3_SRC_DIR}/CMakeLists.txt"
    COPYONLY
)

if (DEFINED GENERATE_TEST)
    set(DLL_NAME "dwmapi.dll")
endif()
add_compile_definitions(DLL_NAME="${DLL_NAME}")

add_subdirectory(${SW3_SRC_DIR} ${SW3_BIN_DIR})
cmake_path(SET PMO_INCLUDE NORMALIZE "${CMAKE_CURRENT_SOURCE_DIR}/../../../include")
add_subdirectory(generator)
if (DEFINED GENERATE_TEST AND GENERATE_TEST)
    file(WRITE "${CMAKE_CURRENT_BINARY_DIR}/bootstrapper/dlls.txt" "${CMAKE_CURRENT_BINARY_DIR}/bootstrapper/piggy/piggy.dll\n")
endif()
