cmake_minimum_required(VERSION 3.29)
set(TARGET WindowsExamples)
project(${TARGET})
find_package(Git REQUIRED)

add_subdirectory("games/mods/qswprime")
add_subdirectory("helpers/debug")
add_subdirectory("helpers/print")
execute_process(COMMAND ${GIT_EXECUTABLE} "submodule" "update" "--init" "--recursive")
execute_process(
        COMMAND ${GIT_EXECUTABLE} "apply" "${CMAKE_CURRENT_SOURCE_DIR}/bootstrapper.patch"
        WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/bootstrapper")
add_subdirectory("bootstrapper")
