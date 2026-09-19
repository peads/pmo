cmake_minimum_required(VERSION 3.29)
project(Example4)
cmake_policy(SET CMP0169 OLD)

find_package(Git REQUIRED)
include(FetchContent)
add_compile_definitions(OPTIMIZATIONS_ON)

FetchContent_Declare(
        syswhispers3
        GIT_REPOSITORY https://github.com/klezVirus/SysWhispers3.git
)

FetchContent_Populate(syswhispers3)
FetchContent_GetProperties(
        syswhispers3
        SOURCE_DIR SW3_SRC_DIR
        BINARY_DIR SW3_BIN_DIR
        POPULATED SW3_IS_POPULATED
)

configure_file(
    "${CMAKE_CURRENT_SOURCE_DIR}/syswhispers3.cmake"
    "${SW3_SRC_DIR}/CMakeLists.txt"
    COPYONLY
)

if (VERBOSE)
    add_compile_definitions(IS_VERBOSE)
endif()

if (NOT DEFINED DLL_NAME OR DEFINED GENERATE_TEST)
    set(DLL_NAME "dwmapi.dll")
endif()

set(CMAKE_C_STANDARD_LIBRARIES "" CACHE STRING "Standard libraries for C" FORCE)
set(CMAKE_CXX_STANDARD_LIBRARIES "" CACHE STRING "Standard libraries for C++" FORCE)

if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    set(LIBS "libvcruntimed.lib" "libcmtd.lib" "ucrtd.lib" "kernel32.lib" "msvcprtd.lib")
else()
    set(LIBS "libvcruntime.lib" "libcmt.lib" "ucrt.lib" "kernel32.lib" "msvcprt.lib")
endif()

add_subdirectory(${SW3_SRC_DIR} ${SW3_BIN_DIR})
cmake_path(SET PMO_INCLUDE NORMALIZE "${CMAKE_CURRENT_SOURCE_DIR}/../../../include")
include_directories("${PMO_INCLUDE}" "${SW3_SRC_DIR}")
add_subdirectory(generator)
if (DEFINED GENERATE_TEST AND GENERATE_TEST)
    add_subdirectory(piggy)
    file(WRITE "${CMAKE_CURRENT_BINARY_DIR}/bootstrapper/dlls.txt" "${CMAKE_CURRENT_BINARY_DIR}/piggy/piggy.dll\n")
endif()
