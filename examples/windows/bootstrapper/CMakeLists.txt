cmake_minimum_required(VERSION 3.29)
project(Example4)
cmake_policy(SET CMP0169 OLD)

find_package(Git REQUIRED)
include(FetchContent)

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

set(CMAKE_C_STANDARD_LIBRARIES "" CACHE STRING "Standard libraries for C" FORCE)
set(CMAKE_CXX_STANDARD_LIBRARIES "" CACHE STRING "Standard libraries for C++" FORCE)

if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    set(LIBS "libvcruntimed.lib" "libcmtd.lib" "ucrtd.lib" "kernel32.lib" "msvcprtd.lib")
else()
    set(LIBS "libvcruntime.lib" "libcmt.lib" "ucrt.lib" "kernel32.lib" "msvcprt.lib")
endif()

#if (GENERATE_BOOTSTRAP)
#    if (NOT DEFINED BOOTSTRAPPER_BUILD_DIR)
#        set(BOOTSTRAPPER_BUILD_DIR "${CMAKE_CURRENT_SOURCE_DIR}/build-bootstrapper")
#    endif()
#    add_executable(bootstrapper main.cpp)
#    target_compile_definitions(bootstrapper PRIVATE
#            IS_BOOTSTRAP="${GENERATE_BOOTSTRAP}"
#            OUT_PATH="${BOOTSTRAPPER_BUILD_DIR}"
#    )
#    target_link_options(bootstrapper PRIVATE "/NODEFAULTLIB" "/MACHINE:X64")
#    target_link_libraries(bootstrapper PRIVATE "${SW3_BIN_DIR}/syscalls.lib")
#    target_link_libraries(bootstrapper PRIVATE "${LIBS}")
#else()
#    enable_language(ASM_NASM)
#    if (NOT EXISTS "${OUT_PATH}/bootstrap.txt")
#        message(FATAL_ERROR "Please build and run the generator half first.")
#    else()
#        file(STRINGS "${OUT_PATH}/bootstrap.txt" FILE_STEM)
#        list(GET FILE_STEM 0 FILE_STEM)
#    endif()
#    add_library(${FILE_STEM} SHARED main.cpp "${OUT_PATH}/${FILE_STEM}.asm")
#    set_target_properties(${FILE_STEM} PROPERTIES
#            LINK_FLAGS "/DEF:\"${OUT_PATH}/${FILE_STEM}.def\""
#    )
#    target_compile_definitions(${FILE_STEM} PRIVATE OUT_PATH="${OUT_PATH}")
#    target_link_options(${FILE_STEM} PRIVATE "/NODEFAULTLIB" "/MACHINE:X64")
#    target_link_libraries(${FILE_STEM} PRIVATE "${SW3_BIN_DIR}/syscalls.lib")
#    target_link_libraries(${FILE_STEM} PRIVATE "${LIBS}")
#    if (GENERATE_TEST)
#        add_executable(dllloader dllloader.cpp)
#        add_subdirectory(piggy "${CMAKE_CURRENT_SOURCE_DIR}/piggy")
#        file(WRITE "${OUT_PATH}/dlls.txt" "${CMAKE_CURRENT_SOURCE_DIR}/piggy/piggy.dll\n")
#        target_link_options(dllloader PRIVATE "/NODEFAULTLIB" "/MACHINE:X64")
#        target_link_libraries(dllloader PRIVATE "${SW3_BIN_DIR}/syscalls.lib")
#        target_link_libraries(dllloader PRIVATE "${LIBS}")
#    endif()
#endif()

add_subdirectory(${SW3_SRC_DIR} ${SW3_BIN_DIR})
cmake_path(SET PMO_INCLUDE NORMALIZE "${CMAKE_CURRENT_SOURCE_DIR}/../../../include")
include_directories("${PMO_INCLUDE}" "${SW3_SRC_DIR}") #${CMAKE_CURRENT_BINARY_DIR})# "${CMAKE_CURRENT_SOURCE_DIR}/SysWhispers3")
add_subdirectory(generator)
if (DEFINED GENERATE_TEST AND GENERATE_TEST)
    add_subdirectory(piggy)
    file(WRITE "${CMAKE_CURRENT_BINARY_DIR}/bootstrapper/dlls.txt" "${CMAKE_CURRENT_SOURCE_DIR}/piggy/piggy.dll\n")
endif()

#add_subdirectory(bootstrapper)
#execute_process(
#        COMMAND ${CMAKE_COMMAND} -S "${SW3_SRC_DIR}" -B "${SW3_BIN_DIR}"
#        RESULT_VARIABLE config_result
#)
#
#if(NOT config_result EQUAL 0)
#    message(FATAL_ERROR "Configuration of subproject failed!")
#endif()
#execute_process(
#        COMMAND ${CMAKE_COMMAND} --build "${SW3_BIN_DIR}"
#        RESULT_VARIABLE build_result
#)
#if(NOT build_result EQUAL 0)
#    message(FATAL_ERROR "Build of subproject failed!")
#endif()
