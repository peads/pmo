cmake_minimum_required(VERSION 3.29)
project(syscalls NONE)
include("${CMAKE_SCRIPT_INCLUDES_PATH}/bashexecute.cmake")
find_package(Python REQUIRED COMPONENTS Interpreter)

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
executeBashCommand(CMD "${Example4_SOURCE_DIR}/masmtonasm.sh" ARGS "${CMAKE_CURRENT_SOURCE_DIR}/${SW3_NAME}-asm.x64.asm")
cmake_path(SET C_FILE NORMALIZE "${CMAKE_CURRENT_SOURCE_DIR}/${SW3_NAME}.c")
executeBashCommand(CMD "-c" ARGS "sed -i 's/stdio/stdlib/g' ${C_FILE}")