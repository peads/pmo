cmake_minimum_required(VERSION 3.29)
project(bootstrapper CXX C)
message(STATUS ${CMAKE_SYSTEM_PROCESSOR})
if (CMAKE_SYSTEM_PROCESSOR MATCHES "(arm64|aarch64|ARM64)")
#        OR MSVC_C_ARCHITECTURE_ID STREQUAL "ARM64" OR CMAKE_ASM_COMPILER_TARGET STREQUAL "arm64-pc-windows-msvc")

    if (CMAKE_CXX_COMPILER_ID MATCHES "Clang")
        enable_language(ASM)
    endif()
#    set(CMAKE_ASM_FLAGS "${CMAKE_ASM_FLAGS} --target=arm64-pc-windows-msvc")
#    add_compile_options(--target=arm64-pc-windows-msvc)
#    add_link_options(/machine:arm64)
else()
    enable_language(ASM_NASM)
endif()

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_C_STANDARD_LIBRARIES "" CACHE STRING "Standard libraries for C" FORCE)
set(CMAKE_CXX_STANDARD_LIBRARIES "" CACHE STRING "Standard libraries for C++" FORCE)
#set(LIBS "")
#set(CMAKE_SCRIPT_INCLUDES_PATH "${CMAKE_CURRENT_SOURCE_DIR}/../../../../cmake")
#include("${CMAKE_SCRIPT_INCLUDES_PATH}/populatelibs.cmake")
#populateLibs(LIBS)

if (NOT EXISTS "${OUT_PATH}/bootstrap.txt")
    message(WARNING "Please build and run the generator half first.")
else ()
    file(STRINGS "${OUT_PATH}/bootstrap.txt" FILE_STEM)
    list(GET FILE_STEM 0 FILE_STEM)

    if (NOT (DEFINED INCLUDE_SYSWHISPERS AND INCLUDE_SYSWHISPERS))
        add_library(${FILE_STEM} SHARED "${CMAKE_CURRENT_SOURCE_DIR}/main.cpp" "${OUT_PATH}/${FILE_STEM}.asm")
    else()
        add_library(${FILE_STEM} SHARED
                "${CMAKE_CURRENT_SOURCE_DIR}/main.cpp"
                "${OUT_PATH}/${FILE_STEM}.asm"
                "${SW3_SRC_DIR}/syscalls-asm.x64.asm"
                "${SW3_SRC_DIR}/syscalls.c"
#                "${OUT_PATH}/${FILE_STEM}.def"
        )
    endif()
if (DEFINED STANDALONE_BOOTSTRAPPER AND STANDALONE_BOOTSTRAPPER)
    cmake_path(SET PMO_INCLUDE NORMALIZE "${CMAKE_CURRENT_SOURCE_DIR}/../../../../include")
else()
    add_dependencies(${FILE_STEM} bootstrapper-generator)
endif()
    target_include_directories(${FILE_STEM} PRIVATE "${PMO_INCLUDE}" "${SW3_SRC_DIR}" "${CMAKE_CURRENT_SOURCE_DIR}/../include")
    set_target_properties(${FILE_STEM} PROPERTIES LINK_FLAGS "/DEF:\"${OUT_PATH}/${FILE_STEM}.def\"")
    target_compile_definitions(${FILE_STEM} PRIVATE OUT_PATH="${OUT_PATH}")
    target_link_options(${FILE_STEM} PRIVATE "/subsystem:console")
    target_link_libraries(${FILE_STEM} PRIVATE "${LIBS}")
    set_target_properties(${FILE_STEM} PROPERTIES MSVC_RUNTIME_CHECKS "")
    if (DEFINED GENERATE_TEST)
        add_compile_definitions(DLL_NAME="${CMAKE_CURRENT_BINARY_DIR}/${DLL_NAME}")
        add_subdirectory(piggy)
        add_custom_command(
                TARGET ${FILE_STEM}
                POST_BUILD
                COMMAND ${CMAKE_COMMAND} -E copy
                    "$<TARGET_FILE:${FILE_STEM}>"
                    "${CMAKE_CURRENT_BINARY_DIR}/piggy"
                COMMENT "Copying build output to piggy"
        )
    endif()
endif()
