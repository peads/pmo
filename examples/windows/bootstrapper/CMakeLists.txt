cmake_minimum_required(VERSION 3.29)
project(Example4 CXX C ASM_NASM)
find_package(Git REQUIRED)
include(FetchContent)
set(CMAKE_SCRIPT_INCLUDES_PATH "${CMAKE_CURRENT_SOURCE_DIR}/../../../cmake")
include("${CMAKE_SCRIPT_INCLUDES_PATH}/populatelibs.cmake")
#function(populateLibs LIBS)
#    set(LIBS "libvcruntimed.lib" "libcmtd.lib" "ucrtd.lib" "kernel32.lib" "msvcprtd.lib" PARENT_SCOPE)
#    if(DEFINED CMAKE_BUILD_TYPE AND CMAKE_BUILD_TYPE)
#        string(TOUPPER "${CMAKE_BUILD_TYPE}" uppercase_CMAKE_BUILD_TYPE)
#        if (NOT uppercase_CMAKE_BUILD_TYPE STREQUAL "DEBUG")
#            unset(LIBS)
#            set(LIBS "libvcruntime.lib" "libcmt.lib" "ucrt.lib" "kernel32.lib" "msvcprt.lib" PARENT_SCOPE)
#            add_compile_definitions(OPTIMIZATIONS_ON)
#        endif()
#    endif()
#endfunction()

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

if (DEFINED GENERATE_TEST AND GENERATE_TEST)
    set(DLL_NAME "dwmapi.dll")
    file(WRITE "${CMAKE_CURRENT_BINARY_DIR}/bootstrapper/piggy/dlls.txt" "${CMAKE_CURRENT_BINARY_DIR}/bootstrapper/piggy/piggy.dll\n")
endif()
add_compile_definitions(DLL_NAME="${DLL_NAME}")

add_compile_options(
        "$<$<AND:$<C_COMPILER_ID:MSVC>,$<OR:$<COMPILE_LANGUAGE:C>,$<COMPILE_LANGUAGE:CXX>>>:-Zp8;-c;-nologo;-Gy;-Oi>"
        "$<$<AND:$<NOT:$<CONFIG:Debug>>,$<C_COMPILER_ID:MSVC>,$<COMPILE_LANGUAGE:C>>:-O1;-Os;-GR-;-EHa;-GS>"
)
if (DEFINED DLL_NAME AND DLL_NAME)
    cmake_path(GET DLL_NAME STEM DLL_NAME_STEM)
endif()
set(OUT_PATH "${CMAKE_CURRENT_BINARY_DIR}/bootstrapper")

add_subdirectory(${SW3_SRC_DIR} ${SW3_BIN_DIR})
cmake_path(SET PMO_INCLUDE NORMALIZE "${CMAKE_CURRENT_SOURCE_DIR}/../../../include")
add_subdirectory(generator)

if (DEFINED DLL_NAME)
    file(WRITE "${OUT_PATH}/bootstrap.txt" "${DLL_NAME_STEM}\n")
    file(TOUCH "${OUT_PATH}/${DLL_NAME_STEM}.def")
    file(TOUCH "${OUT_PATH}/${DLL_NAME_STEM}.asm")
    add_subdirectory(bootstrapper)
endif()
