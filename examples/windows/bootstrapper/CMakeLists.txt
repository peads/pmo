cmake_minimum_required(VERSION 3.29)
project(Example4 CXX C ASM_NASM)
function(executeBashCommand cmd args)
    find_program(BASH_EXECUTABLE NAMES bash git-bash HINTS "[HKLM/SOFTWARE/Microsoft/Windows/CurrentVersion;ProgramFilesDir]/Git/usr/bin" REQUIRED)
    cmake_path(GET BASH_EXECUTABLE PARENT_PATH GIT_BASH_USR_BIN)

    #TODO: When cmake 4.4 becomes more common move this ugly garbage to the ENVIRONMENT option
    set(OLD_PATH "$ENV{PATH}")
    set(ENV{PATH} "${GIT_BASH_USR_BIN};$ENV{PATH}")
    execute_process(
            COMMAND "${BASH_EXECUTABLE}" "${cmd}" "${args}"
            COMMAND_ECHO STDOUT
            #TODO e.g.,
            #ENVIRONMENT PATH="${GIT_BASH_USR_BIN};$ENV{PATH}"
    )
    set(ENV{PATH} "${OLD_PATH}")
endfunction()
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

if (DEFINED GENERATE_TEST AND GENERATE_TEST)
    set(DLL_NAME "dwmapi.dll")
    file(WRITE "${CMAKE_CURRENT_BINARY_DIR}/bootstrapper/piggy/dlls.txt" "${CMAKE_CURRENT_BINARY_DIR}/bootstrapper/piggy/piggy.dll\n")
endif()
add_compile_definitions(DLL_NAME="${DLL_NAME}")
add_compile_definitions("$<$<COMPILE_LANGUAGE:C>:CMAKE_C_FLAGS=\"${CMAKE_C_FLAGS} -Zp8 -EHa -nologo\">")
cmake_path(GET DLL_NAME STEM DLL_NAME_STEM)

if (DEFINED DLL_NAME)
    set(OUT_PATH "${CMAKE_CURRENT_BINARY_DIR}/bootstrapper")
endif()

add_subdirectory(${SW3_SRC_DIR} ${SW3_BIN_DIR})
cmake_path(SET PMO_INCLUDE NORMALIZE "${CMAKE_CURRENT_SOURCE_DIR}/../../../include")
add_subdirectory(generator)

if (DEFINED DLL_NAME)
    file(WRITE "${OUT_PATH}/bootstrap.txt" "${DLL_NAME_STEM}\n")
    file(TOUCH "${OUT_PATH}/${DLL_NAME_STEM}.def")
    file(TOUCH "${OUT_PATH}/${DLL_NAME_STEM}.asm")
    add_subdirectory(bootstrapper)
endif()