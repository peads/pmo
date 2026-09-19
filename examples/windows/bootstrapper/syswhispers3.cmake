cmake_minimum_required(VERSION 3.29)
project(syscalls C ASM_MASM)
find_package(Python REQUIRED COMPONENTS Interpreter)

set(CMAKE_C_STANDARD 23)
set(CMAKE_C_STANDARD_LIBRARIES "" CACHE STRING "Standard libraries for C" FORCE)
set(CMAKE_CXX_STANDARD_LIBRARIES "" CACHE STRING "Standard libraries for C++" FORCE)
set(SW3_OPTS "--preset" "common" "-o" "syscalls")

execute_process(
        COMMAND ${Python_EXECUTABLE} "${CMAKE_CURRENT_SOURCE_DIR}/syswhispers.py" ${SW3_OPTS}
        RESULT_VARIABLE SCRIPT_RETURN_CODE
        OUTPUT_VARIABLE SCRIPT_OUTPUT
        ERROR_VARIABLE  SCRIPT_ERROR
        WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}"
        COMMAND_ECHO    STDOUT
)
if(NOT SCRIPT_RETURN_CODE EQUAL 0)
    message(FATAL_ERROR "Python script failed with error: ${SCRIPT_ERROR}")
endif()

#enable_language(ASM_MASM)
add_library(syscalls STATIC "${CMAKE_CURRENT_SOURCE_DIR}/syscalls-asm.x64.asm" "${CMAKE_CURRENT_SOURCE_DIR}/syscalls.c" "${CMAKE_CURRENT_SOURCE_DIR}/syscalls.h")
target_link_options(syscalls PRIVATE /NODEFAULTLIB /MACHINE:X64 /subsystem:console)
target_compile_options(syscalls PRIVATE $<$<COMPILE_LANGUAGE:C>:-Zp8 -EHa>)
target_compile_options(syscalls PRIVATE -nologo)

set_target_properties(syscalls PROPERTIES
        MSVC_RUNTIME_CHECKS ""
)
