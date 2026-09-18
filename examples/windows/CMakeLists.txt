cmake_minimum_required(VERSION 3.29)
set(TARGET WindowsExamples)
project(${TARGET})

add_subdirectory("games/mods")
add_subdirectory("helpers/debug")
add_subdirectory("helpers/print")
#add_subdirectory("bootstrapper") # purposefully omitted due to complexity