if(NOT CMAKE_HOST_WIN32)
    message(WARNING "Just use bash or whatever you prefer normally with `execute_process`, heathen.")
    return()
endif()

find_program(BASH_EXECUTABLE NAMES bash git-bash HINTS "[HKLM/SOFTWARE/Microsoft/Windows/CurrentVersion;ProgramFilesDir]/Git/usr/bin" REQUIRED)

function(executeBashCommand)
    set(options)
    set(oneValueArgs CMD)
    set(multiValueArgs ARGS)
    cmake_parse_arguments(ARG "${options}" "${oneValueArgs}" "${multiValueArgs}" ${ARGN})

    foreach(SRC IN LISTS ARG_ARGS)
        message(STATUS "Found arg: ${SRC}")
    endforeach()
    message(STATUS "${ARG_ARGS}")

    cmake_path(GET BASH_EXECUTABLE PARENT_PATH GIT_BASH_USR_BIN)

    #TODO: When cmake 4.4 becomes more common move this ugly garbage to the ENVIRONMENT option
    set(OLD_PATH "$ENV{PATH}")
    set(ENV{PATH} "${GIT_BASH_USR_BIN};$ENV{PATH}")
    execute_process(
            COMMAND "${BASH_EXECUTABLE}" ${ARG_CMD} ${ARG_ARGS}
            COMMAND_ECHO STDOUT
            RESULT_VARIABLE BASH_SCRIPT_RETURN_CODE
            OUTPUT_VARIABLE BASH_SCRIPT_OUTPUT
            ERROR_VARIABLE  BASH_SCRIPT_ERROR
            #TODO e.g.,
            #ENVIRONMENT PATH="${GIT_BASH_USR_BIN};$ENV{PATH}"
    )
    set(ENV{PATH} "${OLD_PATH}")
    if(NOT BASH_SCRIPT_RETURN_CODE EQUAL 0)
        message(FATAL_ERROR "${cmd} failed with error:\n${BASH_SCRIPT_ERROR}")
    endif()
endfunction()