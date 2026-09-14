cmake_minimum_required(VERSION 3.29)
set(TARGET WindowsExamples)
project(${TARGET})
#set(CMAKE_CXX_STANDARD 23)
#set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_subdirectory("games/mods")
add_subdirectory("helpers/debug")
add_subdirectory("helpers/print")
