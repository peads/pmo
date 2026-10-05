cmake_minimum_required(VERSION 3.29)
project(Example4 CXX)

if (CMAKE_SYSTEM_PROCESSOR MATCHES "(arm64|aarch64|ARM64)")
    set(IS_ARM64 1)
endif()

cmake_path(SET PMO_INCLUDE NORMALIZE "${CMAKE_CURRENT_SOURCE_DIR}/../../../include")

if (NOT (CMAKE_CXX_COMPILER_ID STREQUAL "GNU" OR CMAKE_C_COMPILER_ID STREQUAL "GNU"))
    set(ASM_EXTENSION ".asm")
else()
    set(ASM_EXTENSION ".s")
    execute_process(
            COMMAND "which" "gcc-ar"
            OUTPUT_VARIABLE CMAKE_AR
            OUTPUT_STRIP_TRAILING_WHITESPACE
    )
    execute_process(
            COMMAND "which" "gcc-ranlib"
            OUTPUT_VARIABLE CMAKE_RANLIB
            OUTPUT_STRIP_TRAILING_WHITESPACE
    )
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
endif()

if (DEFINED GENERATE_TEST AND GENERATE_TEST)
    file(WRITE "${OUT_PATH}/piggy/dlls.txt" "${OUT_PATH}/piggy/piggy.dll\n")
endif()

add_subdirectory(generator)

if (DEFINED DLL_NAME AND NOT CROSS_COMPILE_ARM64)
    file(WRITE "${OUT_PATH}/bootstrap.txt" "${DLL_NAME_STEM}\n")
    file(TOUCH "${OUT_PATH}/${DLL_NAME_STEM}.def")
    file(TOUCH "${OUT_PATH}/${DLL_NAME_STEM}${ASM_EXTENSION}")
    add_subdirectory(bootstrapper)
endif()
