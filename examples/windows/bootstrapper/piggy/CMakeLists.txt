cmake_minimum_required(VERSION 3.29)
project(piggy C)
set(CMAKE_C_STANDARD 23)
set(CMAKE_C_STANDARD_LIBRARIES "" CACHE STRING "Standard libraries for C" FORCE)
set(CMAKE_CXX_STANDARD_LIBRARIES "" CACHE STRING "Standard libraries for C++" FORCE)

add_library(piggy SHARED piggy.c)

if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    set(LIBS "libvcruntimed.lib" "libcmtd.lib" "ucrtd.lib" "kernel32.lib" "msvcprtd.lib")
else()
    set(LIBS "libvcruntime.lib" "libcmt.lib" "ucrt.lib" "kernel32.lib" "msvcprt.lib")
endif()

target_link_options(piggy PRIVATE "/NODEFAULTLIB" "/MACHINE:X64")
target_link_libraries(piggy PRIVATE "${CMAKE_SOURCE_DIR}/SysWhispers3/build/syscalls.lib")
target_link_libraries(piggy PRIVATE "${LIBS}")
