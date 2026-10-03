cmake_minimum_required(VERSION 3.29)
project(Example4 CXX C)
if (CMAKE_SYSTEM_PROCESSOR MATCHES "(arm64|aarch64|ARM64)")
    set(IS_ARM64 1)
endif()
add_compile_definitions("$<$<NOT:$<CONFIG:Debug>>:OPTIMIZATIONS_ON>")
cmake_path(SET PMO_INCLUDE NORMALIZE "${CMAKE_CURRENT_SOURCE_DIR}/../../../include")

if (CMAKE_CXX_COMPILER_ID STREQUAL "GNU" OR CMAKE_C_COMPILER_ID STREQUAL "GNU")
    set(ASM_EXTENSION ".s")
else()
    set(ASM_EXTENSION ".asm")
endif()

if (DEFINED DLL_NAME AND DLL_NAME)
    cmake_path(GET DLL_NAME STEM DLL_NAME_STEM)
endif()

if(DEFINED CROSS_COMPILE_ARM64 AND CROSS_COMPILE_ARM64)
    if (NOT CMAKE_CXX_COMPILER_ID MATCHES "Clang|GNU")
        message(FATAL_ERROR "Only clang and gcc are supported")
    endif()
    if (NOT (DEFINED DLL_NAME AND DLL_NAME))
        message(FATAL_ERROR "Please provide a valid library to bootstrap via `-DDLL_NAME=\"...\"`")
    endif()
    enable_language(ASM)
    if(NOT CMAKE_LINKER)
        message(FATAL_ERROR "Linker path variable (CMAKE_LINKER) is not populated yet.")
    else()
        find_program(RESOLVED_LINKER_PATH NAMES ${CMAKE_LINKER})

        if(NOT RESOLVED_LINKER_PATH)
            cmake_path(ABSOLUTE_PATH CMAKE_LINKER BASE_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR} OUTPUT_VARIABLE RESOLVED_LINKER_PATH)
        endif()
    endif()
    set(OUT_PATH "${CMAKE_CURRENT_BINARY_DIR}")
    set(IS_ARM64 1)

    find_library(lcmt libcmt)
    find_library(lcpmt libcpmt)
    find_library(lvcr libvcruntime)
    find_library(lucrt ucrt)

    set(llist "${lcmt}" "${lcpmt}" "${lvcr}" "${lucrt}")
    set(paths "")
    foreach(target IN LISTS llist)
        cmake_path(GET target PARENT_PATH libr_path)
        cmake_path(GET target FILENAME libr)
        cmake_path(SET target NORMALIZE "${libr_path}/../arm64/${libr}")
        list(APPEND paths "${target}")
    endforeach()

    add_subdirectory(generator)
else()

    find_package(Git REQUIRED)
    include(FetchContent)
    set(CMAKE_SCRIPT_INCLUDES_PATH "${CMAKE_CURRENT_SOURCE_DIR}/../../../cmake")
    include("${CMAKE_SCRIPT_INCLUDES_PATH}/populatelibs.cmake")
    add_compile_definitions("$<$<NOT:$<CONFIG:Debug>>:OPTIMIZATIONS_ON>")
    if (DEFINED INCLUDE_SYSWHISPERS AND INCLUDE_SYSWHISPERS)
        set(CMAKE_C_STANDARD_LIBRARIES "" CACHE STRING "Standard libraries for C" FORCE)
        set(CMAKE_CXX_STANDARD_LIBRARIES "" CACHE STRING "Standard libraries for C++" FORCE)
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
        add_link_options("$<$<NOT:$<CXX_COMPILER_ID:GNU>>:/NODEFAULTLIB>")
        set(LIBS "")
        populateLibs(LIBS)
    endif()

    if (DEFINED GENERATE_TEST AND GENERATE_TEST)
        set(DLL_NAME "dwmapi.dll")
        file(WRITE "${CMAKE_CURRENT_BINARY_DIR}/bootstrapper/piggy/dlls.txt" "${CMAKE_CURRENT_BINARY_DIR}/bootstrapper/piggy/piggy.dll\n")
    endif()

    add_compile_options(
            "$<$<AND:$<C_COMPILER_ID:MSVC>,$<OR:$<COMPILE_LANGUAGE:C>,$<COMPILE_LANGUAGE:CXX>>>:-nologo;-Gy;-Oi;/EHsc>"
            "$<$<AND:$<NOT:$<CONFIG:Debug>>,$<C_COMPILER_ID:MSVC>,$<COMPILE_LANGUAGE:C>>:-Os;-GR-;-EHa;-GS>"
    )
    if (DEFINED DLL_NAME AND DLL_NAME)
        cmake_path(GET DLL_NAME STEM DLL_NAME_STEM)
    endif()
    set(OUT_PATH "${CMAKE_CURRENT_BINARY_DIR}/bootstrapper")

    if (DEFINED INCLUDE_SYSWHISPERS AND INCLUDE_SYSWHISPERS)
        add_subdirectory(${SW3_SRC_DIR} ${SW3_BIN_DIR})
    endif()

    cmake_path(SET PMO_INCLUDE NORMALIZE "${CMAKE_CURRENT_SOURCE_DIR}/../../../include")
    add_subdirectory(generator)

    if (DEFINED DLL_NAME)
        file(WRITE "${OUT_PATH}/bootstrap.txt" "${DLL_NAME_STEM}\n")
        file(TOUCH "${OUT_PATH}/${DLL_NAME_STEM}.def")
        file(TOUCH "${OUT_PATH}/${DLL_NAME_STEM}${ASM_EXTENSION}")
        add_subdirectory(bootstrapper)
    endif()
endif()
