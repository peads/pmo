cmake_minimum_required(VERSION 3.29)
project(piggy C)
set(CMAKE_C_STANDARD 23)

add_link_options("/MACHINE:X64" "/subsystem:console")
add_library(piggy SHARED piggy.c)
add_executable(dllloader dllloader.c)

set_target_properties(piggy PROPERTIES MSVC_RUNTIME_CHECKS "")
set_target_properties(dllloader PROPERTIES MSVC_RUNTIME_CHECKS "")
