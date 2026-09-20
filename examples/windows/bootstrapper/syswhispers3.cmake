cmake_minimum_required(VERSION 3.29)
project(syscalls C ASM_NASM)
find_package(Python REQUIRED COMPONENTS Interpreter)

#set(CMAKE_C_STANDARD 23)
#set(CMAKE_C_STANDARD_LIBRARIES "" CACHE STRING "Standard libraries for C" FORCE)
#set(CMAKE_CXX_STANDARD_LIBRARIES "" CACHE STRING "Standard libraries for C++" FORCE)
set(SW3_NAME "syscalls")
set(SW3_OPTS "--preset" "common" "-o" ${SW3_NAME})

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
executeBashCommand("${Example4_SOURCE_DIR}/masmtonasm.sh" "${CMAKE_CURRENT_SOURCE_DIR}/${SW3_NAME}-asm.x64.asm")
